#include "filesystem/interface/guard.hpp"

namespace filesystem
{
	struct Environment
	{
		Interface fs;

		Environment() { fs_available = true; }
		~Environment() { fs_available = false; }
	};

	Environment env = {};
	bool fs_available = false;
	Interface& fs = env.fs;

#ifdef RVISA_A
	Mutex fs_mutex;
#endif

}