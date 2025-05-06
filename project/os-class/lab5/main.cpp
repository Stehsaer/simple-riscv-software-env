#include <algorithm>
#include <cstring>
#include <iostream>
#include <print>
#include <queue>
#include <random>
#include <ranges>
#include <thread>
#include <vector>

#include <device/csr-timer.hpp>
#include <file/driver/uart.hpp>
#include <file/interface.hpp>
#include <platform/qemu.hpp>

#ifndef RVISA_A
#error "Requires Atomic extension to work!"
#endif

extern "C"
{
	extern uint8_t _tdata_start, _tdata_size, _tbss_start, _tbss_size;
}

#define SCHEDULER_FCFS false

namespace
{
	namespace config
	{
		constexpr size_t interrupt_interval_ms = 10;
		constexpr size_t stack_size = 2 * 1024 * 1024;
	}

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

	void task_end();

	class Scheduler
	{
		enum class State : uint8_t
		{
			Ready,
			Running,
			Dead
		};

		struct Process_control_block
		{
			uintptr_t pc;
			std::array<uint32_t, 31> registers;
			State state;
			std::unique_ptr<std::array<uint8_t, config::stack_size>> stack;
			std::vector<uint8_t> thread_local_data;
			std::unique_ptr<_reent> reent_ptr;
		};

		std::queue<std::unique_ptr<Process_control_block>> ready_queue;
		std::unique_ptr<Process_control_block> running_process = nullptr;
		std::array<uint32_t, 31> register_buffer;
		_reent* interrupt_handler_impure_ptr;

		std::unique_ptr<Process_control_block> takedown_running_process()
		{
			if (running_process != nullptr)
			{
				auto prev_running = std::move(running_process);

				prev_running->pc = get_mepc();
				std::ranges::copy(register_buffer, prev_running->registers.begin());

				return prev_running;
			}

			return nullptr;
		}

		void push_new_running_process()
		{
			running_process = std::move(ready_queue.front());
			ready_queue.pop();

			set_mepc(running_process->pc);
			std::ranges::copy(running_process->registers, register_buffer.begin());
			running_process->state = State::Running;
		}

	  public:

		Scheduler()
		{
			set_mscratch(register_buffer.data());
			interrupt_handler_impure_ptr = _impure_ptr;
		}

		using Task_func = void();

		void tick()
		{
			_impure_ptr = interrupt_handler_impure_ptr;

			if (!SCHEDULER_FCFS
				|| (SCHEDULER_FCFS && (running_process == nullptr || running_process->state == State::Dead)))
			{
				auto prev_running = takedown_running_process();

				if (prev_running != nullptr && prev_running->state == State::Running)
				{
					prev_running->state = State::Ready;
					ready_queue.push(std::move(prev_running));
				}

				if (ready_queue.empty())
				{
					std::println("No more tasks to run, shutting down...");
					exit(0);
				}

				push_new_running_process();
			}

			_impure_ptr = running_process->reent_ptr.get();
		}

		void add_process(Task_func& task)
		{
			auto context = std::make_unique<Process_control_block>();

			context->pc = reinterpret_cast<uintptr_t>(&task);
			context->state = State::Ready;
			context->stack = std::make_unique<std::array<uint8_t, config::stack_size>>();
			context->thread_local_data = std::vector<uint8_t>((size_t)&_tdata_size + (size_t)&_tbss_size);
			context->reent_ptr = std::make_unique<_reent>(_reent _REENT_INIT());

			std::memcpy(context->thread_local_data.data(), &_tdata_start, (size_t)&_tdata_size);
			std::memset(context->thread_local_data.data() + (size_t)&_tdata_size, 0, (size_t)&_tbss_size);

			context->registers[0] = (uintptr_t)(&task_end);                                     // ra
			context->registers[1] = (uintptr_t)(&(*context->stack->end()) - sizeof(uint32_t));  // sp
			context->registers[2] = get_gp();                                                   // gp
			context->registers[3] = (uintptr_t)context->thread_local_data.data();               // tp

			ready_queue.push(std::move(context));
		}

		void notify_task_end()
		{
			if (running_process != nullptr) running_process->state = State::Dead;
			asm volatile("wfi");
		}
	};

	std::unique_ptr<Scheduler> scheduler = std::make_unique<Scheduler>();

	void task_end()
	{
		scheduler->notify_task_end();
	}

	void init_environment()
	{
		auto& uart = platform::qemu::uart;
		file::fs.mount_device("uart:/", std::make_unique<file::driver::qemu::Uart_driver>(uart));
		freopen("uart:/", "w", stdout);
	}

	extern "C"
	{
		void timer_interrupt()
		{
			scheduler->tick();
			const auto prev_timecmp = csr_get_timecmp();
			csr_set_timecmp(prev_timecmp + platform::qemu::mtime_rate * config::interrupt_interval_ms / 1000);
		}

		void init_interrupt();
	}

	std::atomic<int> id = 0;
}

static void test()
{
	static thread_local std::array<uint32_t, 1000000> v;
	const auto id = ::id.fetch_add(1);

	std::random_device rng;
	std::mt19937 mt19937(std::hash<size_t>()(rng() + id));

	for (int i = 0; i < 5; i++)
	{
		for (auto& item : v) item = mt19937();
		std::ranges::sort(v);

		for (auto i : std::views::iota(0zu, v.size() - 1))
			if (v[i] > v[i + 1])
			{
				std::println("Thread {}: Sort failed!", id);
				continue;
			}

		std::println("Thread {}, attempt {}: Sort done, first element is {}", id, i, v[0]);
	}
}

static void test2()
{
	static thread_local std::array<uint32_t, 2000000> v;
	const auto id = ::id.fetch_add(1);

	std::random_device rng;
	std::mt19937 mt19937(std::hash<size_t>()(rng() + id));

	for (int i = 0; i < 5; i++)
	{
		for (auto& item : v) item = mt19937();
		std::ranges::sort(v);

		for (auto i : std::views::iota(0zu, v.size() - 1))
			if (v[i] > v[i + 1])
			{
				std::println("Thread {}: Sort failed!", id);
				continue;
			}

		std::println("Thread {}, attempt {}: Sort done, first element is {}", id, i, v[0]);
	}
}

int main()
{
	init_environment();

	scheduler->add_process(test);
	scheduler->add_process(test);
	scheduler->add_process(test2);
	scheduler->add_process(test2);

	csr_set_timer(0);
	csr_set_timecmp(1000);
	init_interrupt();
	return 0;
}
