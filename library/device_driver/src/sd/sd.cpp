/*
 * File: FATFS_SD.c
 * Driver Name: [[ FATFS_SD SPI ]]
 * SW Layer:   MIDWARE
 * Author:     Khaled Magdy
 * -------------------------------------------
 * For More Information, Tutorials, etc.
 * Visit Website: www.DeepBlueMbedded.com
 */

#include "driver/sd/sd.hpp"

namespace driver::sd
{
	static volatile DSTATUS stat = STA_NOINIT; /* Disk Status */
	static uint8_t card_type;                  /* Type 0:MMC, 1:SDC, 2:Block addressing */
	static uint8_t power_flag = 0;             /* Power flag */

	//-----[ SD Card Functions ]-----

	class Timer
	{
		uint32_t start;
		uint32_t timeout;

	  public:

		Timer(uint32_t timeout_us) :
			start(userio::get_clock()),
			timeout(timeout_us)
		{
		}

		bool wait() const { return userio::get_clock() - start < timeout; }
	};

	void reset_sd_library()
	{
		stat = STA_NOINIT;
		power_flag = 0;
	}

	/* wait SD ready */
	static uint8_t ready_wait()
	{
		uint8_t res;
		/* timeout 500ms */
		Timer timer(500000);
		/* if SD goes ready, receives 0xFF */
		do {
			res = userio::spi_rx_byte();
		} while ((res != 0xFF) && timer.wait());
		return res;
	}

	/* power on */
	static void power_on()
	{
		uint8_t args[6];
		uint32_t cnt = 0x1FFF;
		/* transmit bytes to wake up */
		userio::deselect();
		for (int i = 0; i < 10; i++)
		{
			userio::spi_tx_byte(0xFF);
		}
		/* slave select */
		userio::select();
		/* make idle state */
		args[0] = CMD0; /* CMD0:GO_IDLE_STATE */
		args[1] = 0;
		args[2] = 0;
		args[3] = 0;
		args[4] = 0;
		args[5] = 0x95;
		userio::spi_tx_buffer(args, sizeof(args));
		/* wait response */
		while ((userio::spi_rx_byte() != 0x01) && cnt)
		{
			cnt--;
		}
		userio::deselect();
		userio::spi_tx_byte(0XFF);
		power_flag = 1;
	}

	/* power off */
	static void power_off()
	{
		power_flag = 0;
	}

	/* check power flag */
	static uint8_t check_power()
	{
		return power_flag;
	}

	/* receive data block */
	static bool rx_data_block(BYTE* buff, UINT len)
	{
		uint8_t token;
		/* timeout 200ms */
		Timer timer(200000);
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
	static bool tx_data_block(const uint8_t* buff, BYTE token)
	{
		uint8_t resp;
		uint8_t i = 0;

		/* wait SD ready */
		if (ready_wait() != 0xFF) return false;

		/* transmit token */
		userio::spi_tx_byte(token);

		/* if it's not STOP token, transmit data */
		if (token != 0xFD)
		{
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
		else
		{
			/* recv buffer clear */
			while (userio::spi_rx_byte() == 0);
			return true;
		}
	}
#endif /* _USE_WRITE */

	/* transmit command */
	static BYTE send_cmd(BYTE cmd, uint32_t arg)
	{
		uint8_t crc, res;

		/* wait SD ready */
		if (ready_wait() != 0xFF) return 0xFF;

		/* transmit command */
		userio::spi_tx_byte(cmd);                  /* Command */
		userio::spi_tx_byte((uint8_t)(arg >> 24)); /* Argument[31..24] */
		userio::spi_tx_byte((uint8_t)(arg >> 16)); /* Argument[23..16] */
		userio::spi_tx_byte((uint8_t)(arg >> 8));  /* Argument[15..8] */
		userio::spi_tx_byte((uint8_t)arg);         /* Argument[7..0] */

		/* prepare CRC */
		if (cmd == CMD0)
			crc = 0x95; /* CRC for CMD0(0) */
		else if (cmd == CMD8)
			crc = 0x87; /* CRC for CMD8(0x1AA) */
		else
			crc = 1;

		/* transmit CRC */
		userio::spi_tx_byte(crc);

		/* Skip a stuff byte when STOP_TRANSMISSION */
		if (cmd == CMD12) userio::spi_rx_byte();

		/* receive response */
		uint8_t n = 10;
		do {
			res = userio::spi_rx_byte();
		} while ((res & 0x80) && --n);

		return res;
	}

	//-----[ user_diskio.c Functions ]-----

	/* initialize SD */
	DSTATUS disk_initialize(BYTE drv)
	{
		uint8_t n, type, ocr[4];
		/* single drive, drv should be 0 */
		if (drv) return STA_NOINIT;
		/* no disk */
		if (stat & STA_NODISK) return stat;
		/* power on */
		power_on();
		/* slave select */
		userio::select();
		/* check disk type */
		type = 0;
		/* send GO_IDLE_STATE command */
		if (send_cmd(CMD0, 0) == 1)
		{
			/* timeout 1 sec */
			Timer timer(1000000);
			/* SDC V2+ accept CMD8 command, http://elm-chan.org/docs/mmc/mmc_e.html */
			if (send_cmd(CMD8, 0x1AA) == 1)
			{
				/* operation condition register */
				for (n = 0; n < 4; n++)
				{
					ocr[n] = userio::spi_rx_byte();
				}
				/* voltage range 2.7-3.6V */
				if (ocr[2] == 0x01 && ocr[3] == 0xAA)
				{
					/* ACMD41 with HCS bit */
					do {
						if (send_cmd(CMD55, 0) <= 1 && send_cmd(CMD41, 1UL << 30) == 0) break;
					} while (timer.wait());

					/* READ_OCR */
					if (timer.wait() && send_cmd(CMD58, 0) == 0)
					{
						/* Check CCS bit */
						for (n = 0; n < 4; n++)
						{
							ocr[n] = userio::spi_rx_byte();
						}

						/* SDv2 (HC or SC) */
						type = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;
					}
				}
			}
			else
			{
				/* SDC V1 or MMC */
				type = (send_cmd(CMD55, 0) <= 1 && send_cmd(CMD41, 0) <= 1) ? CT_SD1 : CT_MMC;
				do {
					if (type == CT_SD1)
					{
						if (send_cmd(CMD55, 0) <= 1 && send_cmd(CMD41, 0) == 0) break; /* ACMD41 */
					}
					else
					{
						if (send_cmd(CMD1, 0) == 0) break; /* CMD1 */
					}
				} while (timer.wait());
				/* SET_BLOCKLEN */
				if (!timer.wait() || send_cmd(CMD16, 512) != 0) type = 0;
			}
		}
		card_type = type;
		/* Idle */
		userio::deselect();
		userio::spi_rx_byte();
		/* Clear STA_NOINIT */
		if (type)
		{
			stat &= ~STA_NOINIT;
		}
		else
		{
			/* Initialization failed */
			power_off();
		}
		return stat;
	}

	/* return disk status */
	DSTATUS disk_status(BYTE drv)
	{
		if (drv) return STA_NOINIT;
		return stat;
	}

	/* read sector */
	DRESULT disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count)
	{
		/* pdrv should be 0 */
		if (pdrv || !count) return RES_PARERR;

		/* no disk */
		if (stat & STA_NOINIT) return RES_NOTRDY;

		/* convert to byte address */
		if (!(card_type & CT_SD2)) sector *= 512;

		userio::select();

		if (count == 1)
		{
			/* READ_SINGLE_BLOCK */
			if ((send_cmd(CMD17, sector) == 0) && rx_data_block(buff, 512)) count = 0;
		}
		else
		{
			/* READ_MULTIPLE_BLOCK */
			if (send_cmd(CMD18, sector) == 0)
			{
				do {
					if (!rx_data_block(buff, 512)) break;
					buff += 512;
				} while (--count);

				/* STOP_TRANSMISSION */
				send_cmd(CMD12, 0);
			}
		}

		/* Idle */
		userio::deselect();
		userio::spi_rx_byte();

		return count ? RES_ERROR : RES_OK;
	}

/* write sector */
#if _USE_WRITE == 1
	DRESULT disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count)
	{
		/* pdrv should be 0 */
		if (pdrv || !count) return RES_PARERR;

		/* no disk */
		if (stat & STA_NOINIT) return RES_NOTRDY;

		/* write protection */
		if (stat & STA_PROTECT) return RES_WRPRT;

		/* convert to byte address */
		if (!(card_type & CT_SD2)) sector *= 512;

		userio::select();

		if (count == 1)
		{
			/* WRITE_BLOCK */
			if ((send_cmd(CMD24, sector) == 0) && tx_data_block(buff, 0xFE)) count = 0;
		}
		else
		{
			/* WRITE_MULTIPLE_BLOCK */
			if (card_type & CT_SD1)
			{
				send_cmd(CMD55, 0);
				send_cmd(CMD23, count); /* ACMD23 */
			}

			if (send_cmd(CMD25, sector) == 0)
			{
				do {
					if (!tx_data_block(buff, 0xFC)) break;
					buff += 512;
				} while (--count);

				tx_data_block(nullptr, 0xFD); /* STOP_TRAN token */
			}
		}

		/* Idle */
		userio::deselect();
		userio::spi_rx_byte();

		return count ? RES_ERROR : RES_OK;
	}
#endif /* _USE_WRITE */

	/* ioctl */
	DRESULT disk_ioctl(BYTE drv, BYTE ctrl, void* buff)
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
			case 0:
				power_off(); /* Power Off */
				res = RES_OK;
				break;
			case 1:
				power_on(); /* Power On */
				res = RES_OK;
				break;
			case 2:
				*(ptr + 1) = check_power();
				res = RES_OK; /* Power Check */
				break;
			default:
				res = RES_PARERR;
			}
		}
		else
		{
			/* no disk */
			if (stat & STA_NOINIT)
			{
				return RES_NOTRDY;
			}
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
				if (ready_wait() == 0xFF) res = RES_OK;
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