#pragma once

#include "module/device/uart.hpp"
#include "module/file/interface.hpp"

namespace file::driver
{
	class Fpga_uart_driver : public file::File_device
	{
		device::Fpga_uart& uart;

	  public:

		Fpga_uart_driver(device::Fpga_uart& uart) :
			uart(uart)
		{
		}

		std::pair<error_t, std::unique_ptr<file::File_descriptor>> open(std::string_view path, int flags, mode_t mode) noexcept
			override;

		error_t stat(std::string_view path, struct stat& info) noexcept override;
		error_t unlink(std::string_view path) noexcept override;
		error_t link(std::string_view oldname, std::string_view newname) noexcept override;
		int rename(std::string_view oldpath, std::string_view newpath) noexcept override;

		class Fd : public file::File_descriptor
		{
			device::Fpga_uart& uart;

		  public:

			Fd(device::Fpga_uart& uart) :
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

	class Qemu_uart_driver : public file::File_device
	{
		device::Qemu_uart& uart;

	  public:

		Qemu_uart_driver(device::Qemu_uart& uart) :
			uart(uart)
		{
		}

		std::pair<error_t, std::unique_ptr<file::File_descriptor>> open(std::string_view path, int flags, mode_t mode) noexcept
			override;

		error_t stat(std::string_view path, struct stat& info) noexcept override;
		error_t unlink(std::string_view path) noexcept override;
		error_t link(std::string_view oldname, std::string_view newname) noexcept override;
		int rename(std::string_view oldpath, std::string_view newpath) noexcept override;

		class Fd : public file::File_descriptor
		{
			device::Qemu_uart& uart;

		  public:

			Fd(device::Qemu_uart& uart) :
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