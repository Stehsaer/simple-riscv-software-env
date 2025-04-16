#include "file/interface/guard.hpp"

namespace file
{
	struct Environment
	{
		Filesystem_interface fs;

		Environment() { fs_available = true; }
		~Environment() { fs_available = false; }
	};

	Environment env = {};
	bool fs_available = false;
	Filesystem_interface& fs = env.fs;
}