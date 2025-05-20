#include <print>

#include "scheduler.hpp"

extern "C"
{
	bool test_and_set(int* dst);
}

class Spinlock
{
	int val;

  public:

	Spinlock() :
		val(0)
	{
	}

	void lock()
	{
		while (!test_and_set(&val))
		{
		}
	}

	void unlock() { val = 0; }
};

static int cheese_burger;
static Spinlock lock;

static void mommy_action()
{
	lock.lock();

	printf("mommy: start to make cheese burger, there are %d cheese burger now\n", cheese_burger);
	// make 10 cheese_burger
	cheese_burger += 10;

	printf("mommy: oh, I have to hang clothes out.\n");

	// hanging clothes out
	sleep(1);
	// done

	printf("mommy: Oh, Jesus! There are %d cheese burgers\n", cheese_burger);

	lock.unlock();
}

static void naughty_son()
{
	lock.lock();

	printf("lxj  : Look what I found! %d cheese burgers!\n", cheese_burger);
	// eat all cheese_burgers out secretly
	cheese_burger -= 10;
	printf("lxj  : Yummy yummy\n");
	// run away as fast as possible

	lock.unlock();
}

int main()
{
	init_environment();

	scheduler->add_process(mommy_action);
	scheduler->add_process(naughty_son);

	csr_set_timecmp(10000);
	init_interrupt();
}