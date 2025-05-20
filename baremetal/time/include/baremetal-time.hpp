#pragma once

#include <cstdint>
#include <sys/types.h>

uint64_t platform_get_usecond();

extern "C"
{
	unsigned int sleep(unsigned int seconds);
	clock_t _times(struct tms* buf);
	int _gettimeofday(struct timeval*, void*);
	int usleep(useconds_t usec);
}