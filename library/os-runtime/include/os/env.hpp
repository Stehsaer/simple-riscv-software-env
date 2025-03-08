#pragma once

#include "cpp-guard.h"
#include "file.hpp"

namespace os
{
	extern bool env_available;

	struct Environment
	{
		filesystem::Filesystem_interface fs;

		Environment() { env_available = true; }
		~Environment() { env_available = false; }
	};

	extern Environment env;
	extern filesystem::Filesystem_interface& fs;
}