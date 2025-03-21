#pragma once

#include <cstdint>

namespace device
{
	struct SPI
	{
		alignas(4) uint16_t divider;       // [Register] Divider, @0x00
		alignas(4) bool cs;                // [Register] Chip Select, @0x04
		alignas(4) volatile uint16_t len;  // [Register] Transmit Length, @0x08

	  private:

		alignas(4) uint8_t __unnamed_padding1 [[maybe_unused]][0x400 - 0x00C];

	  public:

		alignas(4) uint8_t tx[0x800 - 0x400];           // [Buffer] Transmit Buffer, 0x400~0x7FF
		alignas(4) volatile uint8_t rx[0xC00 - 0x800];  // [Buffer] Receieve Buffer, 0x800~0xBFF

	  private:

		alignas(4) uint8_t __unnamed_padding2 [[maybe_unused]][0x1000 - 0xC00];

	  public:

		void set_divider(uint16_t divider) noexcept;
		void select() noexcept;
		void deselect() noexcept;

		void start_transaction(uint16_t len) noexcept;
		bool transaction_is_done() const noexcept;
	};
}