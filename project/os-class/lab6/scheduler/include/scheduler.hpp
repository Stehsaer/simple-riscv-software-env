#pragma once

#include <algorithm>
#include <cstring>
#include <iostream>
#include <print>
#include <queue>
#include <random>
#include <ranges>
#include <thread>
#include <vector>

#include "baremetal-time.hpp"
#include "filesystem.hpp"

namespace config
{
	constexpr size_t interrupt_interval_ms = 10;
	constexpr size_t stack_size = 2 * 1024 * 1024;
}

using Semaphore = void;

enum class Syscall_id
{
	Exit,
	Semaphore_open,
	Semaphore_wait,
	Semaphore_close,
	Semaphore_post,
	Semaphore_get
};

class Scheduler
{
	enum class State : uint8_t
	{
		Ready,
		Running,
		Wait_semaphore,
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

	struct Semaphore_control_block
	{
		std::atomic<uint32_t> count;
		std::queue<std::unique_ptr<Process_control_block>> waiting_processes;
	};

	std::map<Semaphore*, std::unique_ptr<Semaphore_control_block>> semaphores;

	std::queue<std::unique_ptr<Process_control_block>> ready_queue;
	std::unique_ptr<Process_control_block> running_process = nullptr;
	std::array<uint32_t, 31> register_buffer;
	_reent* interrupt_handler_impure_ptr;

	std::unique_ptr<Process_control_block> takedown_running_process(bool increment);
	void push_new_running_process();

  public:

	Scheduler();

	using Task_func = void();

	void tick();
	void tick_syscall();

	void add_process(Task_func& task);

	void shuffle_processes();

	Semaphore* create_new_semaphore(uint32_t init);
};

extern const std::unique_ptr<Scheduler> scheduler;

extern "C"
{
	void init_interrupt();
}

void init_environment();

extern "C"
{
	int do_syscall(int id, int arg1, int arg2, int arg3);

	void scheduler_exit();
	Semaphore* semaphore_open(uint32_t init);
	void semaphore_wait(Semaphore* semaphore);
	void semaphore_post(Semaphore* semaphore);
	bool semaphore_close(Semaphore* semaphore);
	int semaphore_get(Semaphore* semaphore);
}
