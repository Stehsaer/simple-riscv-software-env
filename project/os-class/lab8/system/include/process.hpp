#pragma once

#include "elf.hpp"
#include "memory.hpp"
#include <queue>
#include <variant>

struct Process_registers
{
	uint32_t ra, sp, gp, tp, t0, t1, t2, s0, s1, a0, a1, a2, a3, a4, a5, a6, a7, s2, s3, s4, s5, s6, s7, s8,
		s9, s10, s11, t3, t4, t5, t6;
	uintptr_t pc;
};

class Process_page_table
{
	Page_table* primary_page_table = nullptr;

	Page_table_entry& find_or_allocate_entry(uintptr_t virtual_address);

  public:

	Process_page_table(const Process_page_table&) = delete;
	Process_page_table(Process_page_table&&) = delete;
	Process_page_table& operator=(const Process_page_table&) = delete;
	Process_page_table& operator=(Process_page_table&&) = delete;

	Process_page_table();
	~Process_page_table();

	void assign(uintptr_t virtual_address, uintptr_t physical_address, bool r, bool w, bool x);
	void deassign(uintptr_t virtual_address);

	void set_as_primary() const;
	std::optional<uintptr_t> convert(uintptr_t virtual_address) const;
};

struct Process_memory
{
	uintptr_t heap_begin, heap_end, program_break, thread_pointer;
	std::vector<uintptr_t> heap_pages;

	std::shared_ptr<Program> program;
	std::vector<uintptr_t>
		program_pages;  // Copied program pages, needs to be deallocated after deconstruction

	struct Page
	{
		uintptr_t addr;
		bool r, w, x;
	};

	std::map<uintptr_t, Page> page_mapping;

	Process_memory() = default;
	Process_memory(const Process_memory&) = default;
	Process_memory(Process_memory&&) = delete;
	Process_memory& operator=(const Process_memory&) = default;
	Process_memory& operator=(Process_memory&&) = delete;
	~Process_memory();

	struct Load_program_result
	{
		uintptr_t entry_point;
		uintptr_t thread_pointer;
	};

	// load new program into memory
	Load_program_result load_new_program(std::shared_ptr<Program> new_program);

	std::optional<Page> look_for_page(uintptr_t vm);

  private:

	void deallocate_all_pages();
};

struct Process_stack
{
	std::vector<uintptr_t> stack_pages;  // Arranged from lowest to highest address

	Process_stack();
	~Process_stack();

	void copy_from(const Process_stack& other);
};

struct Process_file
{
	std::string cwd;
	std::array<std::shared_ptr<int>, 64> file_mapping;

	Process_file();
	Process_file(const Process_file&) = delete;
	Process_file(Process_file&&) = delete;
	Process_file& operator=(const Process_file&) = delete;
	Process_file& operator=(Process_file&&) = delete;
	~Process_file() = default;

	void copy_from(const Process_file& other)
	{
		cwd = other.cwd;
		for (size_t i = 0; i < file_mapping.size(); ++i) file_mapping[i] = other.file_mapping[i];
	}
};

struct Process
{
  public:

	enum class State
	{
		Running,
		Ready,
		Waiting,
		Finished,
		Terminated,
	} state;

	Process() = default;
	Process(const Process&) = delete;  // no copy
	Process(Process&&) = delete;
	Process& operator=(const Process&) = delete;
	Process& operator=(Process&&) = delete;
	~Process();

	using Id = uint32_t;

	/* Process ID and Family Info */

	Id id;
	std::optional<Id> parent;  // Parent process ID, std::nullopt if no parent
	std::set<Id> child;

	std::set<Id> waiting;
	size_t waiting_count = 0;

	/* Unique Resources */

	Process_registers registers;
	Process_page_table page_table;
	Process_stack stack;

	/* Sharable Resources */

	std::shared_ptr<Process_memory> memory;
	std::shared_ptr<Process_file> file;

	/* Create */

	static std::optional<Id> clone_from(Process& parent);
	static std::optional<Id> make_new(std::shared_ptr<Program> program);

	/* Change State */

	void set_state(State new_state);

	/* Syscalls */

	void syscall_open();
	void syscall_close();
	void syscall_read();
	void syscall_write();
	void syscall_brk();
	void syscall_fstat();
	void syscall_clone();
	void syscall_wait4();

	/* Handlers & Aux */

	bool handle_page_fault(uintptr_t address);  // Handle page fault, returns true if success
	void activate();
};

extern std::array<std::unique_ptr<Process>, 4096> processes;
std::optional<Process::Id> get_new_process_id();