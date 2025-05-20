#include "filesystem/qemu/virtio.hpp"

namespace filesystem::driver::fatfs::backend::virtio
{
	DSTATUS Media_interface::disk_initialize()
	{
		return RES_OK;
	}

	DSTATUS Media_interface::disk_status()
	{
		return RES_OK;
	}

	DRESULT Media_interface::disk_read(BYTE* buff, LBA_t sector, UINT count)
	{
		return interface.read_sectors(buff, sector, count) ? RES_OK : RES_ERROR;
	}

	DRESULT Media_interface::disk_write(const BYTE* buff, LBA_t sector, UINT count)
	{
		return interface.write_sectors(buff, sector, count) ? RES_OK : RES_ERROR;
	}

	DRESULT Media_interface::disk_ioctl(BYTE cmd, void* buff)
	{
		switch (cmd)
		{
		case GET_SECTOR_COUNT:
			*(LBA_t*)buff = interface.get_device_size();
			return RES_OK;
		case GET_SECTOR_SIZE:
			*(WORD*)buff = 512;
			return RES_OK;
		case GET_BLOCK_SIZE:
			*(DWORD*)buff = 1;
			return RES_OK;
		default:
			return RES_OK;
		}
	}
}