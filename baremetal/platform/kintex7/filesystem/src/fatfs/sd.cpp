/*
 * File: FATFS_SD.c
 * Driver Name: [[ FATFS_SD SPI ]]
 * SW Layer:   MIDWARE
 * Author:     Khaled Magdy
 * -------------------------------------------
 * For More Information, Tutorials, etc.
 * Visit Website: www.DeepBlueMbedded.com
 */

#include "filesystem/kintex7/sd.hpp"

namespace filesystem::driver::fatfs::backend::sd
{
	namespace config
	{
		static constexpr auto wait_busy_timeout_us = 2'000'000u;
	}

	class Timeout
	{
		uint32_t start;
		uint32_t timeout;

	  public:

		Timeout(uint32_t timeout_us) :
			start(userio::get_clock()),
			timeout(timeout_us)
		{
		}

		bool wait() const { return userio::get_clock() - start < timeout; }
	};

	Error_code err = Error_code::OK;

	Card_info card_info = {};

	// Wait for SD card to be ready
	// -- Returns: `true` if ready, `false` if timeout
	static bool ready_wait()
	{
		uint8_t res;

		Timeout timer(500000);

		do {
			res = userio::spi_rx_byte();
		} while ((res != 0xFF) && timer.wait());

		return res == 0xFF;
	}

	// Power On SD Card
	// -- Returns: `false` if failed, `true` if succeeded
	static bool power_on()
	{
		uint8_t args[6] = {CMD0, 0, 0, 0, 0, 0x95};

		userio::deselect();

		// 10*8=80 dummy clocks to wakeup
		for (auto i = 0u; i < 10; i++) userio::spi_tx_byte(0xFF);

		userio::select();

		// GO_IDLE_STATE: CMD0, 0, 0, 0, 0, 0x95
		userio::spi_tx_buffer(args, 6);

		// Wait for 0x01 response
		for (auto i = 0u; i < 32768; i++)
		{
			if (userio::spi_rx_byte() == 0x01) break;

			// Timeout
			if (i == 32767)
			{
				card_info.powered = false;
				userio::deselect();
				return false;
			}
		}

		userio::deselect();
		userio::spi_tx_byte(0XFF);

		card_info.powered = true;
		return true;
	}

	// Power OFF SD Card
	static void power_off()
	{
		card_info.powered = false;
	}

	/* receive data block */
	static bool rx_data_block(uint8_t* buff, UINT len)
	{
		uint8_t token;
		/* timeout 200ms */
		Timeout timer(200000);
		/* loop until receive a response or timeout */
		do {
			token = userio::spi_rx_byte();
		} while ((token == 0xFF) && timer.wait());
		/* invalid response */
		if (token != 0xFE) return false;
		/* receive data */
		userio::spi_rx_buffer(buff, len);
		/* discard CRC */
		userio::spi_rx_byte();
		userio::spi_rx_byte();
		return true;
	}

/* transmit data block */
#if _USE_WRITE == 1
	static bool tx_data_block(const uint8_t* buff, uint8_t token)
	{
		uint8_t resp;
		uint8_t i = 0;

		/* wait SD ready */
		if (!ready_wait()) return false;

		/* transmit token */
		userio::spi_tx_byte(token);

		userio::spi_tx_buffer((uint8_t*)buff, 512);

		/* discard CRC */
		userio::spi_rx_byte();
		userio::spi_rx_byte();

		/* receive response */
		while (i <= 64)
		{
			resp = userio::spi_rx_byte();
			/* transmit 0x05 accepted */
			if ((resp & 0x1F) == 0x05) break;
			i++;
		}

		/* recv buffer clear */
		while (userio::spi_rx_byte() == 0);

		return (resp & 0x1F) == 0x05;
	}
#endif /* _USE_WRITE */

	// Transmit a command to SD card
	// -- Returns: Response uint8_t
	static uint8_t send_cmd(uint8_t cmd, uint32_t arg)
	{
		// Wait for SD card to be ready
		if (!ready_wait()) return 0xFF;

		uint8_t send_data[6]
			= {cmd, (uint8_t)(arg >> 24), (uint8_t)(arg >> 16), (uint8_t)(arg >> 8), (uint8_t)arg};

		if (cmd == CMD0)
			send_data[5] = 0x95;
		else if (cmd == CMD8)
			send_data[5] = 0x87;
		else
			send_data[5] = 0x01;

		// Transmit command
		userio::spi_tx_buffer(send_data, 6);

		// When CMD12, skip a dummy uint8_t
		if (cmd == CMD12) userio::spi_rx_byte();

		// Wait for NCR (No response)
		for (auto i = 0u; i < 65536; i++)
		{
			uint8_t res = userio::spi_rx_byte();
			if (!(res & 0x80)) return res;
		}

		return 0xFF;
	}

	std::optional<uint16_t> query_sd_state()
	{
		userio::select();

		if (!ready_wait())
		{
			userio::deselect();
			return std::nullopt;
		}

		uint8_t resp_h = send_cmd(CMD13, 0);
		uint8_t resp_l = userio::spi_rx_byte();

		userio::deselect();
		userio::spi_rx_byte();

		if ((resp_h & 0x80) != 0) return std::nullopt;

		return (resp_h << 8) | resp_l;
	}

	static bool wait_byte(uint8_t byte, uint32_t timeout_us = 200'000)
	{
		Timeout timeout(timeout_us);

		while (timeout.wait())
			if (userio::spi_rx_byte() == byte) return true;

		return false;
	}

	static bool read_single_block(uint8_t* buff, uint32_t sector)
	{
		if (send_cmd(CMD17, sector) != 0)
		{
			err = Error_code::Read_single_cmd17_failed;
			return false;
		}

		if (!wait_byte(0xFE, config::wait_busy_timeout_us))
		{
			err = Error_code::Read_wait_busy_timeout;
			return false;
		}

		userio::spi_rx_buffer(buff, 512);

		// Discard CRC
		userio::spi_rx_byte();
		userio::spi_rx_byte();

		return true;
	}

	static bool read_multiple_block(uint8_t* buff, uint32_t sector, uint32_t count)
	{
		if (send_cmd(CMD18, sector) != 0)
		{
			err = Error_code::Read_multiple_cmd18_failed;
			return false;
		}

		for (auto i = 0u; i < count; i++)
		{
			if (!wait_byte(0xFE, config::wait_busy_timeout_us))
			{
				err = Error_code::Read_wait_busy_timeout;
				return false;
			}

			userio::spi_rx_buffer(buff, 512);

			// Discard CRC
			userio::spi_rx_byte();
			userio::spi_rx_byte();

			buff += 512;
		}

		// Stop transmission
		send_cmd(CMD12, 0);

		return true;
	}

	static bool write_single_block(const uint8_t* buff, uint32_t sector)
	{
		if (auto ret = send_cmd(CMD24, sector); ret != 0)
		{
			err = Error_code::Write_single_cmd24_failed;
			return false;
		}

		// Wait for 0xFF response (not busy, ready to accept data)
		if (!wait_byte(0xFF, config::wait_busy_timeout_us))
		{
			err = Error_code::Write_wait_busy_timeout;
			return false;
		}

		userio::spi_tx_byte(0xFE);         // prefix 0xFE
		userio::spi_tx_buffer(buff, 512);  // Actual data

		if (![]
			{
				Timeout timeout(config::wait_busy_timeout_us);
				while (timeout.wait())
					if ((userio::spi_rx_byte() & 0x1F) == 0x05) return true;

				return false;
			}())
		{
			// Terminate transmission
			send_cmd(CMD12, 0);
			err = Error_code::Write_single_wait_respond_timeout;
			return false;
		}

		// 1s timeout
		if (!wait_byte(0xFF, config::wait_busy_timeout_us))
		{
			err = Error_code::Write_single_busy_timeout;
			return false;
		}

		return true;
	}

	static bool write_multiple_block(const uint8_t* buff, uint32_t sector, uint32_t count)
	{
		if (auto ret = send_cmd(CMD25, sector); ret != 0)
		{
			err = Error_code::Write_multiple_cmd25_failed;
			return false;
		}

		for (auto i = 0u; i < count; i++)
		{
			// Wait for 0xFF response
			if (!wait_byte(0xFF, config::wait_busy_timeout_us))
			{
				err = Error_code::Write_wait_busy_timeout;
				return false;
			}

			userio::spi_tx_byte(0xFC);         // prefix 0xFC
			userio::spi_tx_buffer(buff, 512);  // Actual data
			buff += 512;

			if (![]
				{
					Timeout timeout(config::wait_busy_timeout_us);
					while (timeout.wait())
						if ((userio::spi_rx_byte() & 0x1F) == 0x05) return true;
					return false;
				}())
			{
				// Terminate transmission
				send_cmd(CMD12, 0);
				err = Error_code::Write_multiple_wait_respond_timeout;
				return false;
			}

			// 1s timeout
			if (!wait_byte(0xFF, config::wait_busy_timeout_us))
			{
				err = Error_code::Write_multiple_busy_timeout;
				return false;
			}

			if (i == count - 1) userio::spi_tx_byte(0xFD);  // Stop transmission
		}

		return true;
	}

	//-----[ user_diskio.c Functions ]-----

	/* initialize SD */
	DSTATUS disk_initialize(uint8_t drv)
	{
		card_info = {};

		if (drv != 0) return STA_NOINIT;     // Only accepts drive 0
		if (!power_on()) return STA_NOINIT;  // Power on failed

		struct SPI_guard
		{
			SPI_guard() { userio::select(); }

			~SPI_guard()
			{
				userio::deselect();
				userio::spi_rx_byte();
			}
		} guard;

		// GO_IDLE_STATE
		if (send_cmd(CMD0, 0) != 1)  // Initialization failed
			return STA_NOINIT;

		// Detect version
		if (send_cmd(CMD8, 0x1AA) == 1) [[likely]]
		{
			// SDC V2+
			Timeout timeout(1000000);

			// Get OCR
			uint8_t ocr[4];
			userio::spi_rx_buffer(ocr, 4);

			// 2.7~3.6V Not supported
			if (ocr[2] != 0x01 || ocr[3] != 0xAA) return STA_NOINIT;

			// ACMD41 with HCS bit (?)
			do {
				if (send_cmd(CMD55, 0) <= 1 && send_cmd(CMD41, 1UL << 30) == 0) break;
			} while (timeout.wait());

			while (true)
			{
				if (!timeout.wait()) return STA_NOINIT;
				if (auto ret = send_cmd(CMD58, 0); ret == 0)
					break;
				else
				{
					userio::spi_rx_byte();
					userio::spi_rx_byte();
					userio::spi_rx_byte();
					userio::spi_rx_byte();
				}
			}

			userio::spi_rx_buffer(ocr, 4);

			// SC/HC bit
			card_info.type = Card_info::SD2;
			card_info.block_addressing = (ocr[0] & 0x40) != 0;
		}
		else
		{
			Timeout timeout(1000000);

			// SDC V1 or MMC
			card_info.type
				= (send_cmd(CMD55, 0) <= 1 && send_cmd(CMD41, 0) <= 1) ? Card_info::SD1 : Card_info::MMC;

			do {
				if (card_info.type == Card_info::SD1)
				{
					if (send_cmd(CMD55, 0) <= 1 && send_cmd(CMD41, 0) == 0) break;  // ACMD41
				}
				else
				{
					if (send_cmd(CMD1, 0) == 0) break;  // CMD1
				}
			} while (timeout.wait());

			// Set block length to 512
			if (!timeout.wait() || send_cmd(CMD16, 512) != 0) return STA_NOINIT;
		}

		// Success

		card_info.initialized = true;
		card_info.write_protected = false;  // Fixed to false

		return 0;
	}

	/* return disk status */
	DSTATUS disk_status(uint8_t drv)
	{
		if (drv != 0) return STA_NOINIT;
		return card_info.initialized ? 0 : STA_NOINIT;
	}

	/* read sector */
	DRESULT disk_read(uint8_t pdrv, uint8_t* buff, DWORD sector, UINT count)
	{
		if (pdrv != 0 || count == 0) return RES_PARERR;
		if (!card_info.initialized) return RES_NOTRDY;

		// uint8_t addressing
		if (!card_info.block_addressing) sector *= 512;

		userio::select();

		if (count == 1 ? read_single_block(buff, sector) : read_multiple_block(buff, sector, count))
			count = 0;

		/* Idle */
		userio::deselect();
		userio::spi_rx_byte();

		return count > 0 ? RES_ERROR : RES_OK;
	}

/* write sector */
#if _USE_WRITE == 1
	DRESULT disk_write(uint8_t pdrv, const uint8_t* buff, DWORD sector, UINT count)
	{
		if (pdrv != 0 || count == 0) return RES_PARERR;
		if (!card_info.initialized) return RES_NOTRDY;
		if (card_info.write_protected) return RES_WRPRT;

		// uint8_t addressing
		if (!card_info.block_addressing) sector *= 512;

		userio::select();

		// Wait for SD card to be ready
		for (int i = 0; i < 10; i++)
			if (ready_wait()) break;

		if (ready_wait())
		{
			if (count == 1 ? write_single_block(buff, sector) : write_multiple_block(buff, sector, count))
				count = 0;
		}

		/* Idle */
		userio::deselect();
		userio::spi_rx_byte();

		return count > 0 ? RES_ERROR : RES_OK;
	}
#endif /* _USE_WRITE */

	/* ioctl */
	DRESULT disk_ioctl(uint8_t drv, uint8_t ctrl, void* buff)
	{
		DRESULT res;
		uint8_t n, csd[16], *ptr = (uint8_t*)buff;
		WORD csize;

		/* pdrv should be 0 */
		if (drv) return RES_PARERR;
		res = RES_ERROR;

		if (ctrl == CTRL_POWER)
		{
			switch (*ptr)
			{
			case 0:  // Power off chip
				power_off();
				res = RES_OK;
				break;
			case 1:  // Power on chip
				power_on();
				res = RES_OK;
				break;
			case 2:  // Power Check
				*(ptr + 1) = card_info.powered;
				res = RES_OK;
				break;
			default:
				res = RES_PARERR;
			}
		}
		else
		{
			if (!card_info.initialized) return RES_NOTRDY;

			userio::select();
			switch (ctrl)
			{
			case GET_SECTOR_COUNT:
				/* SEND_CSD */
				if ((send_cmd(CMD9, 0) == 0) && rx_data_block(csd, 16))
				{
					if ((csd[0] >> 6) == 1)
					{
						/* SDC V2 */
						csize = csd[9] + ((WORD)csd[8] << 8) + 1;
						*(DWORD*)buff = (DWORD)csize << 10;
					}
					else
					{
						/* MMC or SDC V1 */
						n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
						csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
						*(DWORD*)buff = (DWORD)csize << (n - 9);
					}
					res = RES_OK;
				}
				break;
			case GET_SECTOR_SIZE:
				*(WORD*)buff = 512;
				res = RES_OK;
				break;
			case CTRL_SYNC:
				while (!ready_wait());
				res = RES_OK;
				break;
			case MMC_GET_CSD:
				/* SEND_CSD */
				if (send_cmd(CMD9, 0) == 0 && rx_data_block(ptr, 16)) res = RES_OK;
				break;
			case MMC_GET_CID:
				/* SEND_CID */
				if (send_cmd(CMD10, 0) == 0 && rx_data_block(ptr, 16)) res = RES_OK;
				break;
			case MMC_GET_OCR:
				/* READ_OCR */
				if (send_cmd(CMD58, 0) == 0)
				{
					for (n = 0; n < 4; n++)
					{
						*ptr++ = userio::spi_rx_byte();
					}
					res = RES_OK;
				}
			default:
				res = RES_PARERR;
			}
			userio::deselect();
			userio::spi_rx_byte();
		}
		return res;
	}
}