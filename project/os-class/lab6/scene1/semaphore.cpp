#include <print>

#include "scheduler.hpp"

extern "C"
{
	bool test_and_set(int* dst);
}

static int cheese_burger;

static Semaphore* volatile sem = nullptr;

static void mommy_action()
{
	printf("mommy: start to make cheese burger, there are %d cheese burger now\n", cheese_burger);
	// make 10 cheese_burger
	cheese_burger += 10;

	printf("mommy: oh, I have to hang clothes out.\n");

	// hanging clothes out
	sleep(2);
	// done

	printf("mommy: Oh, Jesus! There are %d cheese burgers\n", cheese_burger);

	semaphore_post(sem);

	printf("mommy: Done!\n");
}

static void naughty_son()
{
	semaphore_wait(sem);

	printf("lxj  : Look what I found! %d cheese burgers!\n", cheese_burger);
	// eat all cheese_burgers out secretly
	cheese_burger -= 10;
	printf("lxj  : Yummy yummy\n");
	// run away as fast as possible
}

int main()
{
	init_environment();

	scheduler->add_process(mommy_action);
	scheduler->add_process(naughty_son);

	sem = scheduler->create_new_semaphore(0);

	csr_set_timecmp(10000);
	init_interrupt();
}