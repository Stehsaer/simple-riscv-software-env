#include "file/driver/uart.hpp"

namespace file::driver::kintex7
{
	std::pair<error_t, std::unique_ptr<file::File_descriptor>> Uart_driver::open(
		std::string_view path,
		int flags [[maybe_unused]],
		mode_t mode [[maybe_unused]]
	) noexcept
	{
		if (path != "") return {ENOENT, nullptr};

		return {0, std::make_unique<Fd>(uart)};
	}

	error_t Uart_driver::stat(std::string_view path, struct stat& info) noexcept
	{
		if (path != "") return ENOENT;

		info.st_mode = S_IFCHR;
		info.st_size = 0;
		info.st_blksize = 256;
		info.st_blocks = 0;

		return 0;
	}

	error_t Uart_driver::unlink(std::string_view path [[maybe_unused]]) noexcept
	{
		return ENOTSUP;
	}

	error_t Uart_driver::link(std::string_view oldname [[maybe_unused]], std::string_view newname [[maybe_unused]]) noexcept
	{
		return ENOTSUP;
	}

	int Uart_driver::rename(std::string_view oldpath [[maybe_unused]], std::string_view newpath [[maybe_unused]]) noexcept
	{
		errno = ENOTSUP;
		return -1;
	}

	off_t Uart_driver::Fd::lseek(off_t offset [[maybe_unused]], int whence [[maybe_unused]]) noexcept
	{
		errno = ESPIPE;
		return -1;
	}

	ssize_t Uart_driver::Fd::read(void* buf, size_t count) noexcept
	{
		char* ptr = static_cast<char*>(buf);

		for (size_t i = 0; i < count; i++)
		{
			while (true)
			{
				if (uart.rx_available()) break;

				if (uart.error())
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

	ssize_t Uart_driver::Fd::write(const void* buf, size_t count) noexcept
	{
		const auto* ptr = static_cast<const char*>(buf);

		for (auto i = 0zu; i < count; i++)
		{
			while (true)
				if (uart.tx_available()) break;

			if (*ptr == 0) break;

			uart.tx = *ptr;

			++ptr;
		}

		return count;
	}

	int Uart_driver::Fd::fsync() noexcept
	{
		return 0;
	}

	error_t Uart_driver::Fd::fstat(struct stat& info) noexcept
	{
		info.st_mode = S_IFCHR;
		info.st_size = 0;
		info.st_blksize = 256;
		info.st_blocks = 0;

		return 0;
	}

	bool Uart_driver::Fd::isatty() noexcept
	{
		return true;
	}

	error_t Uart_driver::Fd::close() noexcept
	{
		return 0;
	}

	int Uart_driver::Fd::ftruncate(off_t length [[maybe_unused]]) noexcept
	{
		errno = EINVAL;
		return -1;
	}
}