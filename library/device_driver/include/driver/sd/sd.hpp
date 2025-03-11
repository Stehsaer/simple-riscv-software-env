/*
 * File: FATFS_SD.h
 * Driver Name: [[ FATFS_SD SPI ]]
 * SW Layer:   MIDWARE
 * Author:     Khaled Magdy
 * -------------------------------------------
 * For More Information, Tutorials, etc.
 * Visit Website: www.DeepBlueMbedded.com
 */

#pragma once

#include "cpp-guard.h"

#include "driver/fat32.hpp"

//-----[ MMC/SDC Commands ]-----
#define CMD0 (0x40 + 0)   /* GO_IDLE_STATE */
#define CMD1 (0x40 + 1)   /* SEND_OP_COND */
#define CMD8 (0x40 + 8)   /* SEND_IF_COND */
#define CMD9 (0x40 + 9)   /* SEND_CSD */
#define CMD10 (0x40 + 10) /* SEND_CID */
#define CMD12 (0x40 + 12) /* STOP_TRANSMISSION */
#define CMD13 (0x40 + 13) /* SEND_STATUS */
#define CMD16 (0x40 + 16) /* SET_BLOCKLEN */
#define CMD17 (0x40 + 17) /* READ_SINGLE_BLOCK */
#define CMD18 (0x40 + 18) /* READ_MULTIPLE_BLOCK */
#define CMD23 (0x40 + 23) /* SET_BLOCK_COUNT */
#define CMD24 (0x40 + 24) /* WRITE_BLOCK */
#define CMD25 (0x40 + 25) /* WRITE_MULTIPLE_BLOCK */
#define CMD41 (0x40 + 41) /* SEND_OP_COND (ACMD) */
#define CMD55 (0x40 + 55) /* APP_CMD */
#define CMD58 (0x40 + 58) /* READ_OCR */

//-----[ MMC Card Types (MMC_GET_TYPE) ]-----
#define CT_MMC 0x01   /* MMC ver 3 */
#define CT_SD1 0x02   /* SD ver 1 */
#define CT_SD2 0x04   /* SD ver 2 */
#define CT_SDC 0x06   /* SD */
#define CT_BLOCK 0x08 /* Block addressing */

#ifdef FS_CFG_READONLY
#define _USE_WRITE 0
#else
#define _USE_WRITE 1
#endif

namespace driver::sd
{
	struct Card_info
	{
		enum Type
		{
			MMC,
			SD1,
			SD2,
		} type = SD2;

		bool block_addressing = false;
		bool write_protected = false;
		bool powered = false;
		bool initialized = false;
	};

	enum class Error_code
	{
		OK = 0,

		Read_wait_busy_timeout,
		Write_wait_busy_timeout,

		Read_single_cmd17_failed,
		Read_multiple_cmd18_failed,

		Write_single_cmd24_failed,
		Write_multiple_cmd25_failed,

		Write_single_wait_respond_timeout,
		Write_multiple_wait_respond_timeout,

		Write_single_busy_timeout,
		Write_multiple_busy_timeout,
	};

	extern Error_code err;
	extern Card_info card_info;

	std::optional<uint16_t> query_sd_state();

	//-----[ Prototypes For All User External Functions ]-----
	DSTATUS disk_initialize(BYTE pdrv);
	DSTATUS disk_status(BYTE pdrv);
	DRESULT disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count);
	DRESULT disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count);
	DRESULT disk_ioctl(BYTE drv, BYTE ctrl, void* buff);

	//-----[ User IO ]-----

	namespace userio
	{
		uint32_t get_clock();

		/* slave select */
		void select();

		/* slave deselect */
		void deselect();

		/* SPI transmit a byte */
		void spi_tx_byte(uint8_t data);

		/* SPI transmit buffer */
		void spi_tx_buffer(const uint8_t* buffer, uint16_t len);

		/* SPI receive a byte */
		uint8_t spi_rx_byte();

		/* SPI receive a byte via pointer */
		void spi_rx_buffer(uint8_t* buff, uint16_t len);
	}

	class SD_media_interface : public fat32::Media_interface
	{
	  public:

		SD_media_interface() = default;

		DSTATUS disk_initialize() override { return driver::sd::disk_initialize(0); }
		DSTATUS disk_status() override { return driver::sd::disk_status(0); }
		DRESULT disk_read(BYTE* buff, LBA_t sector, UINT count) override { return driver::sd::disk_read(0, buff, sector, count); }
		DRESULT disk_write(const BYTE* buff, LBA_t sector, UINT count) override
		{
#ifdef FS_CFG_READONLY
			return RES_WRPRT;
#endif

			return driver::sd::disk_write(0, buff, sector, count);
		}
		DRESULT disk_ioctl(BYTE cmd, void* buff) override { return driver::sd::disk_ioctl(0, cmd, buff); }
	};

	inline void mount_media_handler()
	{
		fat32::media_interface = std::make_unique<SD_media_interface>();
	}
}