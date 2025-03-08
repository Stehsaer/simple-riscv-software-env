#include "device/uart.hpp"

#include <cstring>
#include <sys/unistd.h>

namespace device
{
	void Uart::set_config(uint32_t divisor, Parity parity, Stopbits stopbits) volatile noexcept
	{
		union
		{
			struct
			{
				uint32_t _divisor  : 24;
				Parity _parity     : 3;
				Stopbits _stopbits : 1;
				int padding        : 4;
			} union_struct;
			uint32_t value;
		};

		union_struct._divisor = divisor;
		union_struct._parity = parity;
		union_struct._stopbits = stopbits;

		this->config = value;
	}

	std::pair<error_t, std::unique_ptr<os::filesystem::File_descriptor>> Uart::Device::open(
		std::string_view path,
		int flags,
		mode_t mode
	) noexcept
	{
		if (path != "") return {ENOENT, nullptr};

		return {0, std::make_unique<Fd>(uart)};
	}

	error_t Uart::Device::stat(std::string_view path, struct stat& info) noexcept
	{
		if (path != "") return ENOENT;

		info.st_mode = S_IFCHR;
		info.st_size = 0;
		info.st_blksize = 256;
		info.st_blocks = 0;

		return 0;
	}

	error_t Uart::Device::unlink(std::string_view path) noexcept
	{
		return ENOTSUP;
	}

	error_t Uart::Device::link(std::string_view oldname, std::string_view newname) noexcept
	{
		return ENOTSUP;
	}

	int Uart::Device::rename(std::string_view oldpath, std::string_view newpath) noexcept
	{
		errno = ENOTSUP;
		return -1;
	}

	off_t Uart::Fd::lseek(off_t offset, int whence) noexcept
	{
		errno = ESPIPE;
		return -1;
	}

	ssize_t Uart::Fd::read(void* buf, size_t count) noexcept
	{
		char* ptr = static_cast<char*>(buf);

		for (size_t i = 0; i < count; i++)
		{
			while (true)
			{
				const auto status = uart.status;
				if (status.rx_a()) break;

				if (status.err())
				{
					errno = EIO;
					return -1;
				}
			}

			ptr[i] = uart.rx;
			if (ptr[i] == '\n' || ptr[i] == '\r') return i + 1;
		}

		return count;
	}

	ssize_t Uart::Fd::write(const void* buf, size_t count) noexcept
	{
		const auto* ptr = static_cast<const char*>(buf);

		for (int i = 0; i < count; i++)
		{
			while (true)
			{
				const auto status = uart.status;
				if (status.tx_a()) break;
			}

			if (*ptr == 0) break;

			uart.tx = *ptr;

			++ptr;
		}

		return count;
	}

	int Uart::Fd::fsync() noexcept
	{
		return 0;
	}

	error_t Uart::Fd::fstat(struct stat& info) noexcept
	{
		info.st_mode = S_IFCHR;
		info.st_size = 0;
		info.st_blksize = 256;
		info.st_blocks = 0;

		return 0;
	}

	bool Uart::Fd::isatty() noexcept
	{
		return true;
	}

	error_t Uart::Fd::close() noexcept
	{
		return 0;
	}

	int Uart::Fd::ftruncate(off_t length) noexcept
	{
		errno = EINVAL;
		return -1;
	}

}