#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>

#include "cpp-guard.h"

#include "os/file.hpp"

namespace device
{
	class Uart
	{

	  public:

		alignas(4) char tx;
		alignas(4) char rx;

	  private:

		alignas(4) uint32_t config;

	  public:

		alignas(4) class Status
		{
			uint8_t data;

		  public:

			Status(const volatile Status& status) { data = status.data; }
			Status(const Status& status) = default;

			inline bool rx_a() const noexcept { return data & 0b1; }
			inline bool tx_a() const noexcept { return data & 0b10; }
			inline bool err() const noexcept { return data & 0b100; }
		} status;

		enum class Stopbits
		{
			Bit1 = 0,
			Bit2 = 1
		};

		enum class Parity
		{
			None = 0b000,
			Odd = 0b001,
			Even = 0b010,
			Always1 = 0b100,
			Always0 = 0b010
		};

		void set_config(uint32_t divisor, Parity parity, Stopbits) volatile noexcept;

		class Device : public os::filesystem::File_device
		{
			volatile Uart& uart;

		  public:

			Device(volatile Uart& uart) :
				uart(uart)
			{
			}

			std::pair<error_t, std::unique_ptr<os::filesystem::File_descriptor>> open(
				std::string_view path,
				int flags,
				mode_t mode
			) noexcept override;

			error_t stat(std::string_view path, struct stat& info) noexcept override;
			error_t unlink(std::string_view path) noexcept override;
			error_t link(std::string_view oldname, std::string_view newname) noexcept override;
			int rename(std::string_view oldpath, std::string_view newpath) noexcept override;
		};

		class Fd : public os::filesystem::File_descriptor
		{
			volatile Uart& uart;

		  public:

			Fd(volatile Uart& uart) :
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

		std::unique_ptr<os::filesystem::File_device> get_device() volatile { return std::make_unique<Device>(*this); }
	};

}