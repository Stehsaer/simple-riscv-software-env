#include "filesystem/interface/file.hpp"
#include "filesystem/interface/guard.hpp"

#include <cerrno>

extern "C"
{
	int _rename(const char*, const char*)
	{
		return ENOTSUP;
	}

	int _isatty(int fd)
	{
		return filesystem::fs.isatty(fd);
	}

	void _raise(void) {}

	int _unlink(const char* path)
	{
		return filesystem::fs.unlink(path);
	}

	int _link(const char* oldpath, const char* newpath)
	{
		return filesystem::fs.link(oldpath, newpath);
	}

	int _stat(const char* path, struct stat* stat)
	{
		return filesystem::fs.stat(path, *stat);
	}

	int _fstat(int fd, struct stat* stat)
	{
		return filesystem::fs.fstat(fd, *stat);
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
		return filesystem::fs.close(fd);
	}

	int _open(const char* path, int flag, mode_t mode)
	{
		int fd;
		errno = filesystem::fs.open(fd, path, flag, mode);

		return errno == 0 ? fd : -1;
	}

	int _write(int fd, const void* data, size_t size)
	{
		return filesystem::fs.write(fd, data, size);
	}

	_off_t _lseek(int fd, _off_t offset, int whence)
	{
		return filesystem::fs.lseek(fd, offset, whence);
	}

	int _read(int fd, void* data, size_t size)
	{
		return filesystem::fs.read(fd, data, size);
	}

	int _dup(int fd)
	{
		return filesystem::fs.dup(fd);
	}

	int _dup2(int fd, int new_fd)
	{
		return filesystem::fs.dup2(fd, new_fd);
	}

	int mkdir(const char* pathname [[maybe_unused]], mode_t mode [[maybe_unused]])
	{
		errno = ENOTSUP;
		return -1;
	}

	int rmdir(const char* pathname [[maybe_unused]])
	{
		errno = ENOTSUP;
		return -1;
	}
}