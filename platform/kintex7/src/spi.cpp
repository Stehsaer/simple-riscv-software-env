#include "device/spi.hpp"

namespace device
{
	void SPI::set_divider(uint16_t divider) noexcept
	{
		this->divider = divider;
	}

	void SPI::select() noexcept
	{
		cs = true;
	}

	void SPI::deselect() noexcept
	{
		cs = false;
	}

	void SPI::start_transaction(uint16_t len) noexcept
	{
		this->len = len;
	}

	bool SPI::transaction_is_done() const noexcept
	{
		return len == 0;
	}
}