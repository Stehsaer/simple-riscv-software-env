#pragma once

#include "file.hpp"

#ifdef RVISA_A
#include <mutex.hpp>
#endif

namespace file
{
	extern bool fs_available;
	extern Filesystem_interface& fs;

#ifdef RVISA_A
	extern Mutex fs_mutex;
#endif
}