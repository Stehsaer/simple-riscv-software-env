#pragma once

#include <ctime>

extern "C"
{
	int _system(const char*);
	int _rename(const char*, const char*);
	int _isatty(int);
	clock_t _times(struct tms*);
	int _gettimeofday(struct timeval*, void*);
	void _raise(void);
	int _unlink(const char*);
	int _link(const char*, const char*);
	int _stat(const char*, struct stat*);
	int _fstat(int, struct stat*);
	void* _sbrk(ptrdiff_t);
	pid_t _getpid(void);
	int _kill(int, int);
	void _exit(int);
	int _close(int);
	int _open(const char*, int, mode_t);
	int _write(int, const void*, size_t);
	_off_t _lseek(int, _off_t, int);
	int _read(int, void*, size_t);
	int _dup(int);
	int _dup2(int, int);
}