#include "os/syscall.hpp"
#include "os/env.hpp"
#include <cerrno>

static char* current_heap_end = nullptr;

extern "C"
{
	extern struct
	{
		void *const heap_start, *const max_heap_end, *const periph_start;
	} platform_config;

	int _system(const char*)
	{
		return ENOTSUP;
	}

	int _rename(const char*, const char*)
	{
		return ENOTSUP;
	}

	int _isatty(int fd)
	{
		return os::fs.isatty(fd);
	}

	void _raise(void) {}

	int _unlink(const char* path)
	{
		return os::fs.unlink(path);
	}

	int _link(const char* oldpath, const char* newpath)
	{
		return os::fs.link(oldpath, newpath);
	}

	int _stat(const char* path, struct stat* stat)
	{
		return os::fs.stat(path, *stat);
	}

	int _fstat(int fd, struct stat* stat)
	{
		return os::fs.fstat(fd, *stat);
	}

	void* _sbrk(ptrdiff_t incr)
	{
		if (current_heap_end == nullptr) current_heap_end = (char*)platform_config.heap_start;

		char* const prev_heap_end = current_heap_end;

		current_heap_end += incr;

		if (current_heap_end < (char*)platform_config.heap_start || current_heap_end > (char*)platform_config.max_heap_end)
		{
			errno = ENOMEM;
			return (caddr_t)-1;
		}

		return (caddr_t)prev_heap_end;
	}

	pid_t _getpid(void)
	{
		return 0;
	}

	int _kill(int, int)
	{
		return ENOTSUP;
	}

	int _close(int fd)
	{
		return os::fs.close(fd);
	}

	int _open(const char* path, int flag, mode_t mode)
	{
		int fd;
		errno = os::fs.open(fd, path, flag, mode);

		return errno == 0 ? fd : -1;
	}

	int _write(int fd, const void* path, size_t mode)
	{
		return os::fs.write(fd, path, mode);
	}

	_off_t _lseek(int fd, _off_t offset, int whence)
	{
		return os::fs.lseek(fd, offset, whence);
	}

	int _read(int fd, void* data, size_t size)
	{
		return os::fs.read(fd, data, size);
	}

	int _dup(int fd)
	{
		return os::fs.dup(fd);
	}

	int _dup2(int fd, int new_fd)
	{
		return os::fs.dup2(fd, new_fd);
	}
}