#include "allocator.hpp"
#include "elf.hpp"
#include "env.hpp"
#include "platform.hpp"
#include "process.hpp"

int main()
{
	initialize_env();
	printk("System initialized successfully.\n");

	FILE* file = fopen("disk:/a.out", "r");
	if (file == nullptr)
	{
		printk("Failed to open the executable file.\n");
		return 1;
	}

	auto program_result = Program::load(file);
	fclose(file);

	if (!program_result.has_value())
	{
		printk("Failed to load the executable file.\n");
		return 1;
	}

	auto process_result = Process::make_new(std::move(program_result.value()));
	if (!process_result.has_value())
	{
		printk("Failed to create a new process.\n");
		return 1;
	}

	enable_timer_interrupt();

	while (true);
}