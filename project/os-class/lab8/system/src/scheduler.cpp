#include "env.hpp"
#include "process-queue.hpp"
#include "process.hpp"
#include "syscall.h"
#include "trap.hpp"

#include <algorithm>

static uint32_t tick_interval = 10000000;

Scheduler_queue scheduler_queue;

static void cleanup_terminated_processes()
{
	for (int i = 1; i < processes.size(); ++i)
	{
		if (processes[i] == nullptr) continue;

		if (processes[i]->state == Process::State::Terminated)
		{
			processes[i].reset();  // Clean up the process
			printk("Process %u cleaned up\n", i);
		}
	}
}

static bool schedule_next_process()
{
	if (scheduler_queue.running.has_value() && processes[scheduler_queue.running.value()] != nullptr
		&& processes[scheduler_queue.running.value()]->state == Process::State::Running)
		processes[scheduler_queue.running.value()]->set_state(Process::State::Ready);

	scheduler_queue.running.reset();

	if (scheduler_queue.ready.empty()) return false;

	const auto result = scheduler_queue.ready.front();
	scheduler_queue.ready.pop();

	KERNEL_ASSERT(
		processes[result] != nullptr,
		"Process with ID %u is null, logic error in scheduler!",
		result
	);

	processes[result]->set_state(Process::State::Running);

	return true;
}

extern "C" void cpp_trap_handler(uintptr_t mcause, uintptr_t mtval)
{
	const bool is_interrupt = (mcause & 0x80000000) != 0;
	const uintptr_t code = mcause & 0x7FFFFFFF;

	if (scheduler_queue.running.has_value())
	{
		auto& current_process = processes[scheduler_queue.running.value()];

		if (is_interrupt)
		{
			switch ((Interrupt_code)code)
			{
			case Interrupt_code::Timer_interrupt_m:
				break;
			default:
				KERNEL_ERROR("Unexpected interrupt code: %lu", code);
			}
		}
		else
		{
			switch ((Exception_code)code)
			{
			case Exception_code::Ecall_s:
			{
				switch ((Syscall)current_process->registers.a7)
				{
				case Syscall::Exit:
					printk(
						"Process %u exited with code %d\n",
						current_process->id,
						current_process->registers.a0
					);
					current_process->set_state(Process::State::Finished);
					break;
				case Syscall::Fstat:
					current_process->syscall_fstat();
					break;
				case Syscall::Brk:
					current_process->syscall_brk();
					break;
				case Syscall::Read:
					current_process->syscall_read();
					break;
				case Syscall::Open:
					current_process->syscall_open();
					break;
				case Syscall::Close:
					current_process->syscall_close();
					break;
				case Syscall::Write:
					current_process->syscall_write();
					break;
				case Syscall::Wait4:
					current_process->syscall_wait4();
					break;
				case Syscall::Clone:
					current_process->syscall_clone();
					break;
				default:
					printk("Syscall not implemented, exiting\n");
					exit(0);
				}
				if (current_process) current_process->registers.pc += 4;
				break;
			}
			case Exception_code::Load_page_fault:
			case Exception_code::Store_page_fault:
			case Exception_code::Instruction_page_fault:
			{
				uintptr_t fault_address;
				asm volatile("csrr %0, mtval" : "=r"(fault_address));

				const auto handle_result = current_process->handle_page_fault(fault_address);

				if (!handle_result)
				{
					printk(
						"Failed to handle page fault in process %u at address %p\n",
						current_process->id,
						(void*)fault_address
					);
					current_process->set_state(Process::State::Finished);
				}

				break;
			}
			default:
				printk("Unexpected exception with code %u, pc=0x%x\n", code, current_process->registers.pc);
				current_process->set_state(Process::State::Finished);
			}
		}
	}
	else
	{
		if (!is_interrupt)
		{
			uintptr_t mepc;
			asm volatile("csrr %0, mepc" : "=r"(mepc));

			KERNEL_ERROR("Unknown exception %u, pc=0x%x\n", code, mepc);
			abort();
		}
	}

	const auto current_time = csr_get_timer();
	csr_set_timecmp(current_time + tick_interval);

	cleanup_terminated_processes();
	if (!schedule_next_process())
	{
		printk("No ready process ID found, shutting down the system.\n");

		const auto zombie_processes = std::ranges::count_if(
			processes | std::views::drop(2),
			[](const std::unique_ptr<Process>& process)
			{ return process && process->state == Process::State::Finished; }
		);
		printk("Total zombie processes: %u\n", zombie_processes);

		exit(0);
	}
}