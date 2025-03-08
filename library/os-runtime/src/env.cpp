#include "os/env.hpp"

namespace os
{
	Environment env = {};
	bool env_available = false;
	filesystem::Filesystem_interface& fs = env.fs;
}