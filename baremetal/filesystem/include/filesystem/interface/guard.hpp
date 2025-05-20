#pragma once

#include "file.hpp"

#ifdef RVISA_A
#include <mutex.hpp>
#endif

namespace filesystem
{
	extern bool fs_available;
	extern Interface& fs;

#ifdef RVISA_A
	extern Mutex fs_mutex;
#endif
}