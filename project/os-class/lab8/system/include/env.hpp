#pragma once

#include <baremetal-time.hpp>
#include <filesystem/interface.hpp>
#include <platform.hpp>

extern device::virtio::IO* virtio;

void initialize_env();
void deinitialize_env();
void print_raw(const char* str);
void printk(const char* format, ...);

void enable_timer_interrupt();

extern bool serial_initialized;

#define KERNEL_ERROR(message, ...)                                                                           \
	{                                                                                                        \
		if (serial_initialized)                                                                              \
			printf("%s (%d): " message "\n", __FILE__, __LINE__, ##__VA_ARGS__);                             \
		else                                                                                                 \
			print_raw(message);                                                                              \
		abort();                                                                                             \
	}

#ifdef _DEBUG
#define KERNEL_ASSERT(condition, message, ...)                                                               \
	{                                                                                                        \
		if (!(condition)) [[unlikely]]                                                                       \
		{                                                                                                    \
			if (serial_initialized)                                                                          \
				printf("%s (%d): " message "\n", __FILE__, __LINE__, ##__VA_ARGS__);                         \
			else                                                                                             \
				print_raw(message);                                                                          \
			abort();                                                                                         \
		}                                                                                                    \
	}
#else
#define KERNEL_ASSERT(condition, message, ...)                                                               \
	{                                                                                                        \
	}
#endif
