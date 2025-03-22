#include "module/device/kintex7/spi.hpp"

namespace device::kintex7
{
	void SPI::set_divider(uint16_t divider) noexcept
	{
		this->divider = divider;
	}

	void SPI::select() noexcept
	{
		cs = false;
	}

	void SPI::deselect() noexcept
	{
		cs = true;
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