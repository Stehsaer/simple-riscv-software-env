#pragma once

#include <cerrno>
#include <sys/stat.h>
#include <unistd.h>

extern "C"
{

	caddr_t _sbrk(int incr);
	int _write(int file, char* ptr, int len);
	int _close(int file);
	int _fstat(int file, struct stat* st);
	int _open(const char* name, int flags, int mode);
	int _isatty(int file);
	int _lseek(int file, int ptr, int dir);
	int _read(int file, char* ptr, int len);
	int _kill(int pid, int sig);
	int _getpid(void);
	int _unlink(const char* name);
}
