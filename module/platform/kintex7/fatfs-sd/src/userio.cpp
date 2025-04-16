#include "file/driver/fatfs/backend/sd.hpp"
#include "platform/kintex7.hpp"
#include <cstring>

namespace file::driver::fatfs::backend::sd::userio
{
	using namespace platform;

	uint32_t get_clock()
	{
		return static_cast<uint32_t>(kintex7::clock.get_timer() * 1000 / kintex7::frequency_khz);
	}

	/* slave select */
	void select()
	{
		kintex7::spi_1.select();
	}

	/* slave deselect */
	void deselect()
	{
		kintex7::spi_1.deselect();
	}

	/* SPI transmit a byte */
	void spi_tx_byte(uint8_t data)
	{
		kintex7::spi_1.tx[0] = data;
		kintex7::spi_1.start_transaction(1);
		while (!kintex7::spi_1.transaction_is_done());
	}

	/* SPI transmit buffer */
	void spi_tx_buffer(const uint8_t* buffer, uint16_t len)
	{
		if (len == 0) [[unlikely]]
			return;

		std::memcpy(kintex7::spi_1.tx, buffer, len);
		kintex7::spi_1.start_transaction(len);
		while (!kintex7::spi_1.transaction_is_done());
	}

	/* SPI receive a byte */
	uint8_t spi_rx_byte()
	{
		kintex7::spi_1.tx[0] = 0xFF;
		kintex7::spi_1.start_transaction(1);
		while (!kintex7::spi_1.transaction_is_done());
		return kintex7::spi_1.rx[0];
	}

	/* SPI receive a byte via pointer */
	void spi_rx_buffer(uint8_t* buff, uint16_t len)
	{
		if (len == 0) [[unlikely]]
			return;

		std::memset(kintex7::spi_1.tx, 0xFF, len);
		kintex7::spi_1.start_transaction(len);
		while (!kintex7::spi_1.transaction_is_done());

		std::memcpy(buff, (uint8_t*)kintex7::spi_1.rx, len);
	}
}