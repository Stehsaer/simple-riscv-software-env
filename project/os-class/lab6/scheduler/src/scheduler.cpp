#include <algorithm>
#include <cstring>
#include <iostream>
#include <print>
#include <queue>
#include <random>
#include <ranges>
#include <thread>
#include <vector>

#include "scheduler.hpp"

#ifndef RVISA_A
#error Requires Atomic extension to work!
#endif

extern "C"
{
	extern uint8_t _tdata_start, _tdata_size, _tbss_start, _tbss_size;
}

namespace
{
	uintptr_t get_mepc()
	{
		uintptr_t epc;
		asm volatile("csrr %0, mepc" : "=r"(epc));
		return epc;
	}

	void set_mepc(uintptr_t epc)
	{
		asm volatile("csrw mepc, %0" : : "r"(epc));
	}

	void set_mscratch(void* ptr)
	{
		asm volatile("csrw mscratch, %0" : : "r"(ptr));
	}

	uintptr_t get_gp()
	{
		uintptr_t gp;
		asm volatile("mv %0, gp" : "=r"(gp));
		return gp;
	}

	extern "C"
	{
		void timer_interrupt()
		{
			scheduler->tick();
			const auto prev_time = csr_get_timer();
			csr_set_timecmp(prev_time + platform::mtime_rate * config::interrupt_interval_ms / 1000);
		}

		void syscall_interrupt()
		{
			scheduler->tick_syscall();
			const auto prev_time = csr_get_timer();
			csr_set_timecmp(prev_time + platform::mtime_rate * config::interrupt_interval_ms / 1000);
		}
	}
}

extern const std::unique_ptr<Scheduler> scheduler = std::make_unique<Scheduler>();

std::unique_ptr<Scheduler::Process_control_block> Scheduler::takedown_running_process(bool increment)
{
	if (running_process != nullptr)
	{
		auto prev_running = std::move(running_process);

		prev_running->pc = get_mepc();
		if (increment) prev_running->pc += 4;

		std::ranges::copy(register_buffer, prev_running->registers.begin());

		return prev_running;
	}

	return nullptr;
}

void Scheduler::push_new_running_process()
{
	for (auto& [_, semaphore] : semaphores)
	{
		while (semaphore->count > 0)
		{
			if (semaphore->waiting_processes.empty()) break;

			auto process = std::move(semaphore->waiting_processes.front());
			semaphore->waiting_processes.pop();
			semaphore->count--;
			process->state = State::Ready;
			ready_queue.push(std::move(process));
		}
	}

	if (running_process == nullptr)
	{
		if (ready_queue.empty())
		{
			for (const auto& [_, semaphore] : semaphores)
				if (!semaphore->waiting_processes.empty())
				{
					std::println("Deadlock detected, shutting down...");
					exit(0);
				}

			std::println("No more tasks to run, shutting down...");
			exit(0);
		}

		running_process = std::move(ready_queue.front());
		ready_queue.pop();
	}

	set_mepc(running_process->pc);
	std::ranges::copy(running_process->registers, register_buffer.begin());
	running_process->state = State::Running;
}

Scheduler::Scheduler()
{
	set_mscratch(register_buffer.data());
	interrupt_handler_impure_ptr = _impure_ptr;
}

void Scheduler::tick()
{
	_impure_ptr = interrupt_handler_impure_ptr;

	auto prev_running = takedown_running_process(false);

	if (prev_running != nullptr)
	{
		switch (prev_running->state)
		{
		case State::Running:
			prev_running->state = State::Ready;
			ready_queue.push(std::move(prev_running));
			break;
		default:
			break;
		}
	}

	push_new_running_process();

	_impure_ptr = running_process->reent_ptr.get();
}

void Scheduler::tick_syscall()
{
	_impure_ptr = interrupt_handler_impure_ptr;

	auto prev_running = takedown_running_process(true);

	if (prev_running == nullptr)
	{
		std::println("Logic error in syscall, no running process!");
		exit(0);
	}

	Syscall_id syscall_id = static_cast<Syscall_id>(prev_running->registers[9]);  // a0
	uint32_t arg1 = static_cast<int>(prev_running->registers[10]);
	uint32_t arg2 = static_cast<int>(prev_running->registers[11]);
	uint32_t arg3 = static_cast<int>(prev_running->registers[12]);
	int& return_value = *reinterpret_cast<int*>(&prev_running->registers[9]);

	switch (syscall_id)
	{
	case Syscall_id::Exit:
		break;
	case Syscall_id::Semaphore_open:
	{
		std::unique_ptr<Semaphore_control_block> new_semaphore = std::make_unique<Semaphore_control_block>();
		new_semaphore->count = arg1;
		return_value = (uintptr_t)new_semaphore.get();
		semaphores.emplace((Semaphore*)return_value, std::move(new_semaphore));

		prev_running->state = State::Running;
		running_process = std::move(prev_running);
		break;
	}
	case Syscall_id::Semaphore_close:
	{
		auto find_sem = semaphores.find((Semaphore*)arg1);

		if (find_sem == semaphores.end())
		{
			std::println("Error at Semaphore_close: semaphore not found!");
			exit(0);
		}

		if (!find_sem->second->waiting_processes.empty())
		{
			return_value = 0;
			prev_running->state = State::Running;
			running_process = std::move(prev_running);
			break;
		}

		return_value = 1;
		semaphores.erase(find_sem);

		prev_running->state = State::Running;
		running_process = std::move(prev_running);
		break;
	}
	case Syscall_id::Semaphore_post:
	{
		auto find_sem = semaphores.find((Semaphore*)arg1);

		if (find_sem == semaphores.end())
		{
			std::println("Error at Semaphore_post: semaphore not found!");
			exit(0);
		}

		find_sem->second->count++;
		prev_running->state = State::Running;
		running_process = std::move(prev_running);

		break;
	}
	case Syscall_id::Semaphore_wait:
	{
		auto find_sem = semaphores.find((Semaphore*)arg1);
		if (find_sem == semaphores.end())
		{
			std::println("Error at Semaphore_wait: semaphore not found!");
			exit(0);
		}
		prev_running->state = State::Wait_semaphore;
		find_sem->second->waiting_processes.push(std::move(prev_running));
		break;
	}
	case Syscall_id::Semaphore_get:
	{
		auto find_sem = semaphores.find((Semaphore*)arg1);
		if (find_sem == semaphores.end())
		{
			std::println("Error at Semaphore_get: semaphore not found!");
			exit(0);
		}

		prev_running->state = State::Running;
		return_value = find_sem->second->count;
		running_process = std::move(prev_running);

		break;
	}
	default:
		std::println("No such syscall!");
		exit(0);
	}

	push_new_running_process();

	_impure_ptr = running_process->reent_ptr.get();
}

void Scheduler::add_process(Task_func& task)
{
	auto context = std::make_unique<Process_control_block>();

	context->pc = reinterpret_cast<uintptr_t>(&task);
	context->state = State::Ready;
	context->stack = std::make_unique<std::array<uint8_t, config::stack_size>>();
	context->thread_local_data = std::vector<uint8_t>((size_t)&_tdata_size + (size_t)&_tbss_size);
	context->reent_ptr = std::make_unique<_reent>(_reent _REENT_INIT());

	std::memcpy(context->thread_local_data.data(), &_tdata_start, (size_t)&_tdata_size);
	std::memset(context->thread_local_data.data() + (size_t)&_tdata_size, 0, (size_t)&_tbss_size);

	context->registers[0] = (uintptr_t)(&scheduler_exit);                               // ra
	context->registers[1] = (uintptr_t)(&(*context->stack->end()) - sizeof(uint32_t));  // sp
	context->registers[2] = get_gp();                                                   // gp
	context->registers[3] = (uintptr_t)context->thread_local_data.data();               // tp

	ready_queue.push(std::move(context));
}

void init_environment()
{
	auto& uart = platform::uart;
	filesystem::fs.mount_device("uart:/", std::make_unique<filesystem::driver::Serial>(uart));
	freopen("uart:/", "w", stdout);
}

extern "C"
{
	void scheduler_exit()
	{
		do_syscall((int)Syscall_id::Exit, 0, 0, 0);
	}

	Semaphore* semaphore_open(uint32_t init)
	{
		return (Semaphore*)do_syscall((int)Syscall_id::Semaphore_open, (int)init, 0, 0);
	}

	void semaphore_wait(Semaphore* semaphore)
	{
		do_syscall((int)Syscall_id::Semaphore_wait, (uintptr_t)semaphore, 0, 0);
	}

	void semaphore_post(Semaphore* semaphore)
	{
		do_syscall((int)Syscall_id::Semaphore_post, (uintptr_t)semaphore, 0, 0);
	}

	bool semaphore_close(Semaphore* semaphore)
	{
		return do_syscall((int)Syscall_id::Semaphore_close, (uintptr_t)semaphore, 0, 0);
	}

	int semaphore_get(Semaphore* semaphore)
	{
		return do_syscall((int)Syscall_id::Semaphore_get, (uintptr_t)semaphore, 0, 0);
	}
}

void Scheduler::shuffle_processes()
{
	std::vector<std::unique_ptr<Process_control_block>> temp;
	while (!ready_queue.empty())
	{
		temp.push_back(std::move(ready_queue.front()));
		ready_queue.pop();
	}

	std::shuffle(temp.begin(), temp.end(), std::mt19937(std::random_device{}()));

	for (auto& process : temp)
	{
		process->state = State::Ready;
		ready_queue.push(std::move(process));
	}
}

Semaphore* Scheduler::create_new_semaphore(uint32_t init)
{
	auto new_semaphore = std::make_unique<Semaphore_control_block>();
	new_semaphore->count = init;
	auto semaphore_ptr = new_semaphore.get();
	semaphores.emplace(semaphore_ptr, std::move(new_semaphore));
	return semaphore_ptr;
}
