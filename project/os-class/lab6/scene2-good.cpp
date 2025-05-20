#include <array>
#include <optional>
#include <print>
#include <ranges>

#include "scheduler.hpp"

static constexpr int male_customer_count = 6, female_customer_count = 4, max_cake_count = 5;

static Semaphore* volatile male_semaphore, * volatile female_semaphore;

static void male_customer_thread()
{
	while (true)
	{
		std::println("Waiting for a matcha cake...");
		semaphore_wait(male_semaphore);
		std::println("Male customer consumed a matcha cake.");
		sleep(1);
	}
}

static void female_customer_thread()
{
	while (true)
	{
		std::println("Waiting for a mango cake...");
		semaphore_wait(female_semaphore);
		std::println("Female customer consumed a mango cake.");
		sleep(1);
	}
}

static void waiter_A_thread()
{
	while (true)
	{
		const auto male_cake_now = semaphore_get(male_semaphore);

		if (male_cake_now == 0)
		{
			const auto female_cake_now = semaphore_get(female_semaphore);
			if (male_cake_now + female_cake_now < max_cake_count)
			{
				semaphore_post(male_semaphore);
				std::println("Waiter A 6160 put a matcha cake.");
			}
		}

		sleep(1);
	}
}

static void waiter_B_thread()
{
	while (true)
	{
		const auto female_cake_now = semaphore_get(female_semaphore);

		if (female_cake_now == 0)
		{
			const auto male_cake_now = semaphore_get(male_semaphore);
			if (female_cake_now + male_cake_now < max_cake_count)
			{
				semaphore_post(female_semaphore);
				std::println("Waiter B 6160 put a mango cake.");
			}
		}

		sleep(1);
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

	male_semaphore = scheduler->create_new_semaphore(0);
	female_semaphore = scheduler->create_new_semaphore(0);

	csr_set_timecmp(10000);
	init_interrupt();
}