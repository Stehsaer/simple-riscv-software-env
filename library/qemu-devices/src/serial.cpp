#include "qemu-devices/serial.hpp"

namespace qemu_device
{
	volatile Serial& serial = *reinterpret_cast<Serial*>(0x10000000);

	std::pair<error_t, std::unique_ptr<os::filesystem::File_descriptor>> Serial::Device::open(
		std::string_view path,
		int flags [[maybe_unused]],
		mode_t mode [[maybe_unused]]
	) noexcept
	{
		if (path != "") return {ENOENT, nullptr};

		return {0, std::make_unique<Fd>(serial)};
	}

	error_t Serial::Device::stat(std::string_view path, struct stat& info) noexcept
	{
		if (path != "") return ENOENT;

		info.st_mode = S_IFCHR;
		info.st_size = 0;
		info.st_blksize = 256;
		info.st_blocks = 0;

		return 0;
	}

	error_t Serial::Device::unlink(std::string_view path [[maybe_unused]]) noexcept
	{
		return ENOTSUP;
	}

	error_t Serial::Device::link(std::string_view oldname [[maybe_unused]], std::string_view newname [[maybe_unused]]) noexcept
	{
		return ENOTSUP;
	}

	int Serial::Device::rename(std::string_view oldpath [[maybe_unused]], std::string_view newpath [[maybe_unused]]) noexcept
	{
		errno = ENOTSUP;
		return -1;
	}

	off_t Serial::Fd::lseek(off_t offset [[maybe_unused]], int whence [[maybe_unused]]) noexcept
	{
		errno = ESPIPE;
		return -1;
	}

	ssize_t Serial::Fd::read(void* buf, size_t count) noexcept
	{
		char* ptr = static_cast<char*>(buf);

		for (size_t i = 0; i < count; i++)
		{
			ptr[i] = serial.trx;
			if (ptr[i] == '\n' || ptr[i] == '\r') return i + 1;
		}

		return count;
	}

	ssize_t Serial::Fd::write(const void* buf, size_t count) noexcept
	{
		const auto* ptr = static_cast<const char*>(buf);

		for (size_t i = 0; i < count; i++)
		{
			if (*ptr == 0) break;

			serial.trx = *ptr;

			++ptr;
		}

		return count;
	}

	int Serial::Fd::fsync() noexcept
	{
		return 0;
	}

	error_t Serial::Fd::fstat(struct stat& info) noexcept
	{
		info.st_mode = S_IFCHR;
		info.st_size = 0;
		info.st_blksize = 256;
		info.st_blocks = 0;

		return 0;
	}

	bool Serial::Fd::isatty() noexcept
	{
		return true;
	}

	error_t Serial::Fd::close() noexcept
	{
		return 0;
	}

	int Serial::Fd::ftruncate(off_t length [[maybe_unused]]) noexcept
	{
		errno = EINVAL;
		return -1;
	}
}