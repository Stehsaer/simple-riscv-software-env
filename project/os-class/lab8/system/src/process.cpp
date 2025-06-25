#include "process.hpp"

#include "allocator.hpp"
#include "process-queue.hpp"
#include "register-op.hpp"

#include <cstring>
#include <iostream>

#include <sys/fcntl.h>
#include <sys/unistd.h>

#include <sched.h>

namespace
{
	constexpr size_t stack_size = 1048576 * 2;
}

#pragma region Process_page_table

Process_page_table::Process_page_table()
{
	primary_page_table = (Page_table*)allocator::allocate_page();
}

Process_page_table::~Process_page_table()
{
	KERNEL_ASSERT(primary_page_table != nullptr, "Primary page table is not set, logic error!");

	for (int i = 0; i < 1024; i++)
	{
		auto& entry = (*primary_page_table)[i];
		if (entry.get_priv().v)
		{
			auto* page_table_ptr = (Page_table*)entry.get_address();
			if (page_table_ptr)
			{
				allocator::deallocate_page(uintptr_t(page_table_ptr));
			}
		}
	}

	allocator::deallocate_page(uintptr_t(primary_page_table));
}

Page_table_entry& Process_page_table::find_or_allocate_entry(uintptr_t virtual_address)
{
	check_address_4kb_aligned(virtual_address);

	KERNEL_ASSERT(primary_page_table != nullptr, "Primary page table is not set, logic error!");
	auto& first_page_table_item = (*primary_page_table)[virtual_address >> 22];

	if (!first_page_table_item.get_priv().v)
	{
		const auto new_allocated = allocator::allocate_page();
		auto* new_ptr = (Page_table*)new_allocated;

		first_page_table_item = {new_allocated, {.v = true}};
		std::construct_at(new_ptr);

		return (*new_ptr)[(virtual_address >> 12) & 0b11111'11111];
	}

	auto* page_table_ptr = (Page_table*)first_page_table_item.get_address();
	return (*page_table_ptr)[(virtual_address >> 12) & 0b11111'11111];
}

void Process_page_table::assign(uintptr_t virtual_address, uintptr_t physical_address, bool r, bool w, bool x)
{
	auto& page_item = find_or_allocate_entry(virtual_address);
	check_address_4kb_aligned(physical_address);

	page_item = {
		physical_address,
		{.v = true, .r = r, .w = w, .x = x}
	};
}

void Process_page_table::deassign(uintptr_t virtual_address)
{
	auto& page_item = find_or_allocate_entry(virtual_address);
	page_item = {0, {}};
}

void Process_page_table::set_as_primary() const
{
	KERNEL_ASSERT(primary_page_table != nullptr, "Can't set an empty page table as primary, logic error!");

	primary_page_table->set_as_primary_table();
}

std::optional<uintptr_t> Process_page_table::convert(uintptr_t virtual_address) const
{
	const auto first_stage = (*primary_page_table)[virtual_address >> 22];
	if (!first_stage.get_priv().v) return std::nullopt;  // Invalid address
	const auto* page_table_ptr = (Page_table*)first_stage.get_address();
	if (page_table_ptr == nullptr) return std::nullopt;  // Invalid address
	const auto second_stage = (*page_table_ptr)[(virtual_address >> 12) & 0b11111'11111];
	if (!second_stage.get_priv().v) return std::nullopt;            // Invalid address
	return second_stage.get_address() | (virtual_address & 0xfff);  // Return physical address
}

#pragma endregion

#pragma region Process_memory

void Process_memory::deallocate_all_pages()
{
	for (auto page : heap_pages) allocator::deallocate_page(page);
	for (auto page : program_pages) allocator::deallocate_page(page);

	heap_pages.clear();
	program_pages.clear();
}

Process_memory::Load_program_result Process_memory::load_new_program(std::shared_ptr<Program> new_program)
{
	program = std::move(new_program);
	deallocate_all_pages();

	Load_program_result result;
	result.entry_point = program->entry_point;

	const size_t tls_count = std::ranges::count_if(
		program->segments,
		[](const std::unique_ptr<Program::Segment>& segment)
		{ return segment->type == Program::Segment::Type::Thread_local; }
	);

	if (tls_count > 1) KERNEL_ERROR("Multiple thread-local segments are not supported!");

	uintptr_t last_non_tls_address = 0;

	// Load non-tls segments
	for (const auto& segment : program->segments)
	{
		if (segment->type == Program::Segment::Type::Thread_local) continue;

		uintptr_t page_address = segment->page_address;
		if (segment->w)  // Copy writable pages
		{
			for (const auto& page : segment->pages)
			{
				auto allocated_page = allocator::allocate_page();
				allocator::copy_page(page, allocated_page);
				page_mapping.emplace(
					page_address,
					Page{.addr = allocated_page, .r = segment->r, .w = segment->w, .x = segment->x}
				);
				page_address += 0x1000;  // Move to the next page
				program_pages.push_back(allocated_page);
			}
		}
		else  // Reference non-writable pages
		{
			for (const auto& page : segment->pages)
			{
				page_mapping.emplace(
					page_address,
					Page{.addr = page, .r = segment->r, .w = segment->w, .x = segment->x}
				);
				page_address += 0x1000;  // Move to the next page
			}
		}

		page_address += 0x1000;
		last_non_tls_address = std::max(last_non_tls_address, page_address);
	}

	// Find TLS Segment
	result.thread_pointer = last_non_tls_address;
	auto tls_segment = std::ranges::find_if(
		program->segments,
		[](const std::unique_ptr<Program::Segment>& segment)
		{ return segment->type == Program::Segment::Type::Thread_local; }
	);

	uintptr_t tls_end = result.thread_pointer;

	// Load TLS Segment
	if (tls_segment != program->segments.end())
	{
		for (const auto& page : tls_segment->get()->pages)
		{
			auto allocated_page = allocator::allocate_page();
			allocator::copy_page(page, allocated_page);
			page_mapping.emplace(
				tls_end,
				Page{
					.addr = allocated_page,
					.r = tls_segment->get()->r,
					.w = tls_segment->get()->w,
					.x = tls_segment->get()->x
				}
			);
			tls_end += 0x1000;  // Move to the next page
			program_pages.push_back(allocated_page);
		}
	}

	heap_begin = tls_end;
	heap_end = std::numeric_limits<uintptr_t>::max() - stack_size;  // 2 MiB for heap
	program_break = heap_begin;

	return result;
}

std::optional<Process_memory::Page> Process_memory::look_for_page(uintptr_t vm)
{
	const auto find = page_mapping.find(vm & ~0xfff);
	if (find != page_mapping.end())
		return find->second;  // Return physical address
	else
		return std::nullopt;  // Page not found
}

Process_memory::~Process_memory()
{
	deallocate_all_pages();
}

#pragma endregion

#pragma region Process_stack

Process_stack::Process_stack()
{
	// Allocate stack pages
	for (size_t i = 0; i < stack_size / 0x1000; ++i)
	{
		auto allocated_page = allocator::allocate_page();
		stack_pages.push_back(allocated_page);
	}
}

Process_stack::~Process_stack()
{
	// Deallocate stack pages
	for (auto page : stack_pages) allocator::deallocate_page(page);
	stack_pages.clear();
}

void Process_stack::copy_from(const Process_stack& other)
{
	for (auto [dst, src] : std::views::zip(stack_pages, other.stack_pages)) allocator::copy_page(src, dst);
}

#pragma endregion

#pragma region Process_file

Process_file::Process_file()
{
	file_mapping[0] = std::make_shared<int>(0);
	file_mapping[1] = std::make_shared<int>(1);
	file_mapping[2] = std::make_shared<int>(2);
}

#pragma endregion

#pragma region Process

std::optional<Process::Id> Process::clone_from(Process& parent)
{
	std::unique_ptr<Process> new_process = std::make_unique<Process>();

	// Setup registers
	new_process->registers = parent.registers;

	// Setup stack
	new_process->stack.copy_from(parent.stack);

	// Setup id
	const auto new_process_id = get_new_process_id();
	if (!new_process_id.has_value()) return std::nullopt;
	new_process->id = new_process_id.value();
	new_process->parent = parent.id;

	// Setup file
	new_process->file = parent.file;  // Share file

	// Setup memory
	new_process->memory = parent.memory;  // Share memory space

	new_process->set_state(State::Ready);
	processes[new_process_id.value()] = std::move(new_process);

	return new_process_id;
}

std::optional<Process::Id> Process::make_new(std::shared_ptr<Program> program)
{
	std::unique_ptr<Process> new_process = std::make_unique<Process>();

	// Setup id
	const auto new_process_id = get_new_process_id();
	if (!new_process_id.has_value()) return std::nullopt;
	new_process->id = new_process_id.value();

	// Setup file
	new_process->file = std::make_shared<Process_file>();
	new_process->file->cwd = "disk:/";
	new_process->file->file_mapping[0] = std::make_shared<int>(stdin->_file);
	new_process->file->file_mapping[1] = std::make_shared<int>(stdout->_file);
	new_process->file->file_mapping[2] = std::make_shared<int>(stderr->_file);

	// Setup memory
	new_process->memory = std::make_shared<Process_memory>();
	const auto load_program_result = new_process->memory->load_new_program(std::move(program));

	// Setup Registers
	new_process->registers.pc = load_program_result.entry_point;
	new_process->registers.sp = 0xfffffffc;
	new_process->registers.tp = load_program_result.thread_pointer;

	new_process->set_state(State::Ready);
	processes[new_process_id.value()] = std::move(new_process);

	return new_process_id;
}

Process::~Process()
{
	for (auto child_id : child)
	{
		if (processes[child_id] != nullptr)
			processes[child_id]->parent = this->parent;  // Reassign parent to the child
	}

	if (parent.has_value())
	{
		if (processes[parent.value()] != nullptr)
			processes[parent.value()]->child.erase(this->id);  // Remove this process from parent's children
	}
}

void Process::set_state(State new_state)
{
	switch (new_state)
	{
	case State::Running:
		if (state != State::Ready)
			KERNEL_ERROR(
				"Can't set process state to Running if previous state is not Ready! Kernel Logic Error!"
			);
		activate();
		scheduler_queue.running = id;  // Set this process as running
		break;

	case State::Ready:
		if (state == State::Ready)
			KERNEL_ERROR("Process %u is already in Ready state, can't set it again! Kernel Logic Error!", id);
		scheduler_queue.ready.push(id);  // Add to the ready queue
		break;

	case State::Waiting:
		if (state != State::Running)
			KERNEL_ERROR(
				"Can't set process state to Waiting if previous state is not Running! Kernel Logic Error!"
			);
		break;

	case State::Finished:

		if (id == 1)
		{
			new_state = State::Terminated;  // Special case for init process
			break;
		}

		if (parent.has_value())
		{
			auto& parent_process = processes[parent.value()];
			KERNEL_ASSERT(
				parent_process != nullptr,
				"Parent process %u is null, logic error in process %u!",
				parent.value(),
				id
			);

			if (parent_process->waiting_count > 0 && parent_process->waiting.contains(id))
			{
				parent_process->waiting.erase(id);
				parent_process->waiting_count--;

				if (parent_process->waiting_count == 0)
				{
					printk("Process %u finished, waking up parent %u\n", id, parent.value());

					parent_process->set_state(
						State::Ready
					);  // Wake up parent if no more children are waiting
					parent_process->waiting.clear();
					parent_process->registers.a0 = id;  // Set return value to child ID
				}

				new_state = State::Terminated;  // Set to Terminated state
			}
		}
		break;

	case State::Terminated:
		break;
	}

	state = new_state;
}

void Process::syscall_open()
{
	// Check Available File Descriptor Space
	const auto occupied_count = std::count(file->file_mapping.begin(), file->file_mapping.end(), nullptr);
	if (size_t(occupied_count) >= file->file_mapping.size())
	{
		registers.a0 = -EMFILE;  // Too many open files
		return;
	}

	std::string filename;
	uintptr_t filename_ptr = registers.a0;

	const auto phys_addr_opt = page_table.convert(filename_ptr);
	if (!phys_addr_opt.has_value())
	{
		registers.a0 = -EFAULT;  // Bad address
		return;
	}
	uintptr_t physical_addr = phys_addr_opt.value();

	while (true)
	{
		if ((filename_ptr & 0xfff) == 0)
		{
			const auto phys_addr_opt = page_table.convert(filename_ptr);
			if (!phys_addr_opt.has_value())
			{
				registers.a0 = -EFAULT;  // Bad address
				return;
			}
			physical_addr = phys_addr_opt.value();
		}

		if (*(const char*)physical_addr == 0) break;
		if (filename.length() >= 255)
		{
			registers.a0 = -ENAMETOOLONG;  // Filename too long
			return;
		}

		filename.push_back(*(const char*)physical_addr);
		filename_ptr++;
		physical_addr++;
	}

	const int flags = registers.a1;
	const int mode = registers.a2;

	const int result = open(filename.c_str(), flags, mode);

	if (result < 0)
		registers.a0 = -errno;
	else
	{
		const auto find = std::find(file->file_mapping.begin(), file->file_mapping.end(), nullptr);
		*find = std::shared_ptr<int>(
			new int(result),
			[](const int* fd)
			{
				close(*fd);
				delete fd;
			}
		);
		registers.a0 = std::distance(file->file_mapping.begin(), find);
	}
}

void Process::syscall_close()
{
	const int fd = registers.a0;
	if (fd < 0 || std::cmp_greater(fd, file->file_mapping.size()) || file->file_mapping[fd] == nullptr)
	{
		registers.a0 = -EBADF;  // Bad file descriptor
		return;
	}

	file->file_mapping[fd].reset();  // Close the file descriptor
	registers.a0 = 0;                // Success
}

void Process::syscall_read()
{
	const int fd = registers.a0;
	if (fd < 0 || std::cmp_greater(fd, file->file_mapping.size()) || file->file_mapping[fd] == nullptr)
	{
		registers.a0 = -EBADF;  // Bad file descriptor
		return;
	}

	const int sys_fd = *file->file_mapping[fd];

	uintptr_t start_vm = registers.a1;
	size_t count = registers.a2;

	while (count > 0)
	{
		const auto phys_addr_opt = page_table.convert(start_vm);
		if (!phys_addr_opt.has_value())
		{
			registers.a0 = -EFAULT;  // Bad address
			return;
		}

		char* const buffer = reinterpret_cast<char*>(phys_addr_opt.value());
		const ssize_t result
			= read(sys_fd, buffer, std::min<int>(count, (start_vm + 4096) & ~0xfff - start_vm));

		if (result < 0)
		{
			registers.a0 = -errno;  // Error occurred
			return;
		}

		if (result == 0) break;  // EOF reached

		start_vm += result;
		count -= result;
	}

	registers.a0 = registers.a2 - count;  // Return the number of bytes read
}

void Process::syscall_write()
{
	const int fd = registers.a0;
	if (fd < 0 || std::cmp_greater(fd, file->file_mapping.size()) || file->file_mapping[fd] == nullptr)
	{
		registers.a0 = -EBADF;  // Bad file descriptor
		return;
	}

	const int sys_fd = *file->file_mapping[fd];
	uintptr_t start_vm = registers.a1;
	size_t count = registers.a2;

	while (count > 0)
	{
		const auto phys_addr_opt = page_table.convert(start_vm);
		if (!phys_addr_opt.has_value())
		{
			registers.a0 = -EFAULT;  // Bad address
			return;
		}

		const char* const buffer = reinterpret_cast<const char*>(phys_addr_opt.value());
		const ssize_t result
			= write(sys_fd, buffer, std::min<int>(count, (start_vm + 4096) & ~0xfff - start_vm));

		if (result < 0)
		{
			registers.a0 = -errno;  // Error occurred
			return;
		}

		start_vm += result;
		count -= result;
	}

	registers.a0 = registers.a2 - count;  // Return the number of bytes written
}

void Process::syscall_brk()
{
	const auto new_program_break = registers.a0;
	if (new_program_break >= memory->heap_begin && new_program_break < memory->heap_end)
		memory->program_break = new_program_break;
	registers.a0 = memory->program_break;  // Success

	const auto expected_page_count = (memory->program_break - memory->heap_begin + 0xfff) / 0x1000;

	// Allocate new pages
	if (memory->heap_pages.size() < expected_page_count)
	{
		for (size_t i = memory->heap_pages.size(); i < expected_page_count; ++i)
		{
			const auto allocated_page = allocator::allocate_page();
			memory->heap_pages.push_back(allocated_page);

			memory->page_mapping.emplace(
				memory->heap_begin + i * 0x1000,
				Process_memory::Page{.addr = allocated_page, .r = true, .w = true, .x = false}
			);
		}
	}

	// Deallocate old pages
	if (memory->heap_pages.size() > expected_page_count)
	{
		for (size_t i = expected_page_count; i < memory->heap_pages.size(); ++i)
		{
			allocator::deallocate_page(memory->heap_pages[i]);
			const auto virtual_address = memory->heap_begin + i * 0x1000;
			memory->page_mapping.erase(virtual_address);  // Remove from page mapping
		}

		memory->heap_pages.resize(expected_page_count);
	}
}

void Process::syscall_fstat()
{
	const auto fd = registers.a0;
	if (fd < 0 || std::cmp_greater(fd, file->file_mapping.size()) || file->file_mapping[fd] == nullptr)
	{
		registers.a0 = -EBADF;
		return;
	}

	const int sys_fd = *file->file_mapping[fd];
	struct stat statbuf;
	if (fstat(sys_fd, &statbuf) < 0)
	{
		registers.a0 = -errno;  // Error occurred
		return;
	}

	// copy from physical address (`&statbuf`) to virtual address (`registers.a1`)
	auto current_pm = uintptr_t(&statbuf);
	const auto end_pm = current_pm + sizeof(statbuf);
	uintptr_t current_vm = registers.a1;

	while (current_pm < end_pm)
	{
		const auto pm = page_table.convert(current_vm);
		if (!pm.has_value())
		{
			registers.a0 = -EFAULT;  // Bad address
			return;
		}

		const auto copy_length
			= std::min<size_t>((pm.value() % 4096 + 4096) - pm.value(), end_pm - current_pm);

		memcpy(reinterpret_cast<void*>(pm.value()), reinterpret_cast<const void*>(current_pm), copy_length);
		current_pm += copy_length;
		current_vm += copy_length;
	}
}

void Process::syscall_clone()
{
	// Clone the current process
	const auto new_process_id = clone_from(*this);
	if (!new_process_id.has_value())
	{
		registers.a0 = -ENOMEM;  // Not enough memory to clone
		return;
	}

	auto& new_process = processes[new_process_id.value()];
	new_process->registers.pc += 4;        // Increment PC to next instruction
	child.insert(new_process_id.value());  // Add new process to children

	registers.a0 = new_process_id.value();  // Return new process ID
}

void Process::syscall_wait4()
{
	if (child.empty())
	{
		registers.a0 = -1;
		return;
	}

	waiting.clear();
	waiting_count = 0;

	for (auto child : child)
	{
		KERNEL_ASSERT(
			processes[child] != nullptr,
			"Child process %u is null, logic error in process %u!",
			child,
			id
		);

		if (processes[child]->state == State::Finished)
		{
			registers.a0 = child;                            // Return child ID
			processes[child]->set_state(State::Terminated);  // Set child state to Terminated
			return;
		}
		else
		{
			waiting.insert(child);  // Add child to waiting list
			waiting_count++;
		}
	}

	set_state(State::Waiting);  // Set process state to Waiting
}

bool Process::handle_page_fault(uintptr_t address)
{
	// Is stack
	if (address >= uintptr_t(-stack_size))
	{
		const auto current_page = (address + stack_size) / 0x1000;
		if (current_page >= stack.stack_pages.size())
			KERNEL_ERROR(
				"Unexpected stack page overflow: %zu >= %zu",
				current_page,
				stack.stack_pages.size()
			);

		const auto physical_address = stack.stack_pages[current_page], virtual_address = address & ~0xfff;
		page_table.assign(virtual_address, physical_address, true, true, false);
		return true;
	}

	const auto look_memory_result = memory->look_for_page(address);
	if (look_memory_result.has_value())
	{
		const auto [addr, r, w, x] = look_memory_result.value();
		page_table.assign(address & ~0xfff, addr, r, w, x);

		return true;
	}

	return false;
}

void Process::activate()
{
	page_table.set_as_primary();
	asm volatile("csrw mscratch, %0" : : "r"(&registers));
	set_mstatus_mpp_supervisor();
}

#pragma endregion

std::array<std::unique_ptr<Process>, 4096> processes;

std::optional<Process::Id> get_new_process_id()
{
	for (size_t i = 1; i < processes.size(); ++i)
		if (!processes[i]) return static_cast<Process::Id>(i);

	return std::nullopt;  // No free process ID available
}
