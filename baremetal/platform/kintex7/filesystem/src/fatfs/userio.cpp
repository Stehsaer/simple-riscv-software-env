#include "filesystem/kintex7/sd.hpp"
#include "platform.hpp"
#include <cstring>

namespace filesystem::driver::fatfs::backend::sd::userio
{
	uint32_t get_clock()
	{
		return static_cast<uint32_t>(platform::clock.get_timer() * 1000000 / device::Clock::frequency);
	}

	/* slave select */
	void select()
	{
		platform::spi_1.select();
	}

	/* slave deselect */
	void deselect()
	{
		platform::spi_1.deselect();
	}

	/* SPI transmit a byte */
	void spi_tx_byte(uint8_t data)
	{
		platform::spi_1.tx[0] = data;
		platform::spi_1.start_transaction(1);
		while (!platform::spi_1.transaction_is_done());
	}

	/* SPI transmit buffer */
	void spi_tx_buffer(const uint8_t* buffer, uint16_t len)
	{
		if (len == 0) [[unlikely]]
			return;

		std::memcpy(platform::spi_1.tx, buffer, len);
		platform::spi_1.start_transaction(len);
		while (!platform::spi_1.transaction_is_done());
	}

	/* SPI receive a byte */
	uint8_t spi_rx_byte()
	{
		platform::spi_1.tx[0] = 0xFF;
		platform::spi_1.start_transaction(1);
		while (!platform::spi_1.transaction_is_done());
		return platform::spi_1.rx[0];
	}

	/* SPI receive a byte via pointer */
	void spi_rx_buffer(uint8_t* buff, uint16_t len)
	{
		if (len == 0) [[unlikely]]
			return;

		std::memset(platform::spi_1.tx, 0xFF, len);
		platform::spi_1.start_transaction(len);
		while (!platform::spi_1.transaction_is_done());

		std::memcpy(buff, (uint8_t*)platform::spi_1.rx, len);
	}
}