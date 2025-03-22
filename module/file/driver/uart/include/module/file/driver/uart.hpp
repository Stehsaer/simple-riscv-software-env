#pragma once

#include "module/device/kintex7/uart.hpp"
#include "module/device/qemu/uart.hpp"
#include "module/file/interface.hpp"

namespace file::driver
{
	namespace kintex7
	{
		class Uart_driver : public file::File_device
		{
			device::kintex7::Uart& uart;

		  public:

			Uart_driver(device::kintex7::Uart& uart) :
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
				device::kintex7::Uart& uart;

			  public:

				Fd(device::kintex7::Uart& uart) :
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

	namespace qemu
	{
		class Uart_driver : public file::File_device
		{
			device::qemu::Uart& uart;

		  public:

			Uart_driver(device::qemu::Uart& uart) :
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
				device::qemu::Uart& uart;

			  public:

				Fd(device::qemu::Uart& uart) :
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
}