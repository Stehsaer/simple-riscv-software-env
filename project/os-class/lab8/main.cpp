#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string_view>
#include <sys/unistd.h>

void print(const char* str)
{
	const auto length = strlen(str);
	write(1, str, length);
}

#define SYSCALL_CLONE 220
#define SYSCALL_WAIT4 260

static int do_syscall(int syscall_number, int arg1 = 0, int arg2 = 0, int arg3 = 0)
{
	register int a0 asm("a0") = arg1;
	register int a1 asm("a1") = arg2;
	register int a2 asm("a2") = arg3;
	register int a7 asm("a7") = syscall_number;
	asm volatile("ecall" : "=r"(a0) : "r"(a0), "r"(a1), "r"(a2), "r"(a7));
	return a0;
}

int main()
{
	print("Hello, World! OS Class Lab8 by 23336160!\n");
	const auto id = do_syscall(SYSCALL_CLONE);

	if (id < 0)
	{
		print("Failed to clone process\n");
		return 1;
	}

	if (id == 0)
	{
		print("This is the child process. Doing stuffs...\n");

		for (int i = 0; i < 10000000; i++)
		{
			if (i % 1000000 == 0)
			{
				print("Child process is running...\n");
			}
		}

		exit(0);
	}

	if (id > 0)
	{
		print("This is the parent process, waiting for child to finish...\n");
		while (do_syscall(SYSCALL_WAIT4, id) != id);
		print("Child process finished.\n");
	}

	return 0;
}