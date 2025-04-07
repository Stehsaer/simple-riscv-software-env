#include "module/device/qemu/virtio.hpp"
#include "module/file/driver/fatfs.hpp"

namespace file::driver::fatfs::backend::virtio
{
	class Media_interface : public fatfs::Media_interface
	{
		device::qemu::virtio::Block_device_interface interface;

	  public:

		Media_interface(device::qemu::virtio::IO& io) :
			interface(io)
		{
		}

		DSTATUS disk_initialize() override;
		DSTATUS disk_status() override;
		DRESULT disk_read(BYTE* buff, LBA_t sector, UINT count) override;
		DRESULT disk_write(const BYTE* buff, LBA_t sector, UINT count) override;
		DRESULT disk_ioctl(BYTE cmd, void* buff) override;
	};
}