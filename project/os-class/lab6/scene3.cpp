#include <array>
#include <optional>
#include <print>
#include <ranges>

#include "scheduler.hpp"

static std::array<Semaphore* volatile, 5> chopsticks;
static std::atomic<int> pid_counter = 0;

static void philosopher_thread()
{
	const int pid = pid_counter++;
	std::println("(6160): Philosopher {} is created!", pid);

	while (1)
	{
		std::println("(6160): Philosopher {} THINKING... cogito ergo sum!", pid);
		sleep(1);
		std::println("(6160): Philosopher {} HUNGRY... Looking for left chopstick!", pid);
		semaphore_wait(chopsticks[pid % 5]);
		std::println("(6160): Philosopher {} HUNGRY... Looking for right chopstick!", pid);
		semaphore_wait(chopsticks[(pid + 1) % 5]);
		std::println("(6160): Philosopher {} EATING... Yummy yummy yummy!", pid);
		sleep(1);
		std::println("(6160): Philosopher {} DONE EATING... Releasing right chopstick!", pid);
		semaphore_post(chopsticks[(pid + 1) % 5]);
		std::println("(6160): Philosopher {} DONE EATING... Releasing left chopstick!", pid);
		semaphore_post(chopsticks[pid % 5]);
	}
}

int main()
{
	init_environment();

	for (int i = 0; i < 5; ++i)
	{
		scheduler->add_process(philosopher_thread);
	}

	// Initialize the chopsticks
	for (auto& chopstick : chopsticks)
	{
		chopstick = scheduler->create_new_semaphore(1);
	}

	csr_set_timecmp(10000);
	init_interrupt();

	return 0;
}