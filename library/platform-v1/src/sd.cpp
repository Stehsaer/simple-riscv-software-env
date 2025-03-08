#include "driver/sd/sd.hpp"
#include "platform-v1/periph.hpp"
#include <cstring>

namespace driver::sd::userio
{
	uint32_t get_clock()
	{
		return platform_v1::clock.get_tick32();
	}

	/* slave select */
	void select()
	{
		platform_v1::spi_sd.assert_cs();
	}

	/* slave deselect */
	void deselect()
	{
		platform_v1::spi_sd.deassert_cs();
	}

	/* SPI transmit a byte */
	void spi_tx_byte(uint8_t data)
	{
		platform_v1::spi_sd.tx[0] = data;
		platform_v1::spi_sd.send_bytes(1);
		platform_v1::spi_sd.wait();
	}

	/* SPI transmit buffer */
	void spi_tx_buffer(uint8_t* buffer, uint16_t len)
	{
		memcpy((void*)platform_v1::spi_sd.tx, buffer, len);
		platform_v1::spi_sd.send_bytes(len);
		platform_v1::spi_sd.wait();
	}

	/* SPI receive a byte */
	uint8_t spi_rx_byte()
	{
		constexpr uint8_t dummy = 0xFF;
		platform_v1::spi_sd.tx[0] = dummy;
		platform_v1::spi_sd.send_bytes(1);
		platform_v1::spi_sd.wait();
		return platform_v1::spi_sd.rx[0];
	}

	/* SPI receive a byte via pointer */
	void spi_rx_buffer(uint8_t* buff, uint16_t len)
	{
		constexpr uint8_t dummy = 0xFF;
		memset((void*)platform_v1::spi_sd.tx, dummy, len);
		platform_v1::spi_sd.send_bytes(len);
		platform_v1::spi_sd.wait();
		memcpy(buff, (void*)platform_v1::spi_sd.rx, len);
	}
}

namespace platform_v1::sd
{
	void set_speed(uint32_t rate_khz)
	{
		if (rate_khz == 0)
		{
			spi_sd.divider = 65535;
			return;
		}

		const auto divider = frequency / (rate_khz * 1000);
		if (divider > 65535)
			spi_sd.divider = 65535;
		else
			spi_sd.divider = divider;
	}
}