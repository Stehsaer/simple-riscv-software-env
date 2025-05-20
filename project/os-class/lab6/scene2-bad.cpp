#include <array>
#include <optional>
#include <print>
#include <ranges>

#include "scheduler.hpp"

static constexpr int male_customer_count = 6, female_customer_count = 4, max_cake_count = 5;

static volatile int male_cake_count, female_cake_count;
static volatile bool male_signal;
static volatile bool female_signal;

static void male_customer_thread()
{
	while (true)
	{
		if (male_cake_count == 0)
		{
			male_signal = true;
		}
		else
		{
			std::printf(
				"Male customer consumed a matcha cake. (Cake now: %d/%d)\n",
				male_cake_count,
				female_cake_count
			);
			sleep(1);
			male_cake_count -= 1;
		}
	}
}

static void female_customer_thread()
{
	while (true)
	{
		if (female_cake_count == 0)
		{
			female_signal = true;
		}
		else
		{
			std::printf(
				"Female customer consumed a mango cake. (Cake now: %d/%d)\n",
				male_cake_count,
				female_cake_count
			);
			sleep(1);
			female_cake_count -= 1;
		}
	}
}

static void waiter_A_thread()
{
	while (true)
	{
		if (male_signal)
		{
			while (male_cake_count + female_cake_count >= max_cake_count);
			std::printf(
				"Waiter A 6160 put a matcha cake. (Cake now: %d/%d)\n",
				male_cake_count,
				female_cake_count
			);
			sleep(1);
			male_cake_count += 1;
			male_signal = false;
		}
	}
}

static void waiter_B_thread()
{
	while (true)
	{
		if (female_signal)
		{
			while (male_cake_count + female_cake_count >= max_cake_count);
			std::printf(
				"Waiter B 6160 put a mango cake. (Cake now: %d/%d)\n",
				male_cake_count,
				female_cake_count
			);
			sleep(1);
			female_cake_count += 1;
			female_signal = false;
		}
	}
}

int main()
{
	init_environment();

	for (auto i = 0zu; i < male_customer_count; i++) scheduler->add_process(male_customer_thread);
	for (auto i = 0zu; i < female_customer_count; i++) scheduler->add_process(female_customer_thread);
	scheduler->add_process(waiter_A_thread);
	scheduler->add_process(waiter_B_thread);

	scheduler->shuffle_processes();

	csr_set_timecmp(10000);
	init_interrupt();
}