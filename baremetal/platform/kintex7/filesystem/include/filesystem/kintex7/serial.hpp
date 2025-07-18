#pragma once
#include "device/uart.hpp"
#include "filesystem/interface.hpp"

namespace filesystem::driver
{
	class Serial : public filesystem::Device
	{
		device::Uart& uart;

	  public:

		Serial(device::Uart& uart) :
			uart(uart)
		{
		}

		std::pair<error_t, std::unique_ptr<filesystem::File_descriptor>> open(
			std::string_view path,
			int flags,
			mode_t mode
		) noexcept override;

		error_t stat(std::string_view path, struct stat& info) noexcept override;
		error_t unlink(std::string_view path) noexcept override;
		error_t link(std::string_view oldname, std::string_view newname) noexcept override;
		int rename(std::string_view oldpath, std::string_view newpath) noexcept override;

		class Fd : public filesystem::File_descriptor
		{
			device::Uart& uart;

		  public:

			Fd(device::Uart& uart) :
				uart(uart)
			{
			}

			off_t lseek(off_t offset, int whence) noexcept override;
			ssize_t read(void* buf, size_t count) noexcept override;
			ssize_t write(const void* buf, size_t count) noexcept override;
			error_t fstat(struct stat& info) noexcept override;
			bool isatty() noexcept override;
			error_t close() noexcept override;
			error_t fsync() noexcept override;
			int ftruncate(off_t length) noexcept override;
		};
	};
}
