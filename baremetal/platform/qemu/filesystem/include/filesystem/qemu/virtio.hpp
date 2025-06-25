#pragma once

#include "device/virtio.hpp"
#include "filesystem/driver/fatfs.hpp"

namespace filesystem::driver::fatfs::backend::virtio
{
	class Media_interface : public fatfs::Media_interface
	{
		device::virtio::Block_device_interface interface;

	  public:

		Media_interface(device::virtio::IO& io) :
			interface(io)
		{
		}

		virtual ~Media_interface();

		DSTATUS disk_initialize() override;
		DSTATUS disk_status() override;
		DRESULT disk_read(BYTE* buff, LBA_t sector, UINT count) override;
		DRESULT disk_write(const BYTE* buff, LBA_t sector, UINT count) override;
		DRESULT disk_ioctl(BYTE cmd, void* buff) override;
	};
}