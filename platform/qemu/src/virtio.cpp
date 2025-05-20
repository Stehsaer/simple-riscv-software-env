#include "device/virtio.hpp"
#include <cstring>

namespace device::virtio
{
	static constexpr uint32_t max_retry_count = 20;

	void Interface::assign_virtio(IO& virtio)
	{
		if constexpr (sizeof(uintptr_t) == 4)
		{
			virtio.queue_desc_low = reinterpret_cast<uintptr_t>(descriptors.data());
			virtio.queue_driver_low = reinterpret_cast<uintptr_t>(avail.get());
			virtio.queue_device_low = reinterpret_cast<uintptr_t>(used.get());

			virtio.queue_desc_high = 0;
			virtio.queue_driver_high = 0;
			virtio.queue_device_high = 0;
		}
		else
		{
			virtio.queue_desc_low = reinterpret_cast<uintptr_t>(descriptors.data()) & 0xFFFFFFFF;
			virtio.queue_desc_high = reinterpret_cast<uintptr_t>(descriptors.data()) >> 32;
			virtio.queue_driver_low = reinterpret_cast<uintptr_t>(avail.get()) & 0xFFFFFFFF;
			virtio.queue_driver_high = reinterpret_cast<uintptr_t>(avail.get()) >> 32;
			virtio.queue_device_low = reinterpret_cast<uintptr_t>(used.get()) & 0xFFFFFFFF;
			virtio.queue_device_high = reinterpret_cast<uintptr_t>(used.get()) >> 32;
		}
	}

	void Interface::push_descriptor(uint16_t queue_idx) const
	{
		avail->ring[avail->idx % descriptors.size()] = queue_idx;
		avail->idx++;
	}

	bool Interface::check_used_consumed() const
	{
		return used->idx == avail->idx;
	}

	Block_device_interface::Block_device_interface(IO& virtio) :
		io(virtio),
		buffer(std::make_unique<Aligned_buffer<512>>())
	{
		const auto max_queue_size = io.queue_num_max;
		const auto queue_size = std::min<uint32_t>(max_queue_size, default_queue_size);

		io.select_queue(0);
		io.set_queue_size(queue_size);

		interface = std::make_unique<Interface>(queue_size);
		interface->assign_virtio(io);

		io.confirm_queue_ready();
	}

	uint64_t Block_device_interface::get_device_size()
	{
		uint64_t size;
		std::memcpy(&size, (const void*)&io.config_space[0], sizeof(size));
		return size;
	}

	bool Block_device_interface::read_sectors(uint8_t* buffer, size_t sector_begin, size_t sector_count)
	{
		wait();

		const auto queue_size = interface->size();
		if (queue_size < 3) return false;

		for (auto sector = sector_begin; sector < sector_begin + sector_count; sector++)
		{
			for (uint32_t retry = 0; retry < max_retry_count; retry++)
			{
				Block_request request = Block_request::read(sector);
				volatile uint8_t status;

				interface->descriptors[0].set_object(request);
				interface->descriptors[0].flags = {.next = true, .write = false};
				interface->descriptors[0].next = 1;

				interface->descriptors[1].addr = reinterpret_cast<uintptr_t>(this->buffer->data);
				interface->descriptors[1].len = 512;
				interface->descriptors[1].flags = {.next = true, .write = true};
				interface->descriptors[1].next = 2;

				interface->descriptors[2].set_object(status);
				interface->descriptors[2].len = 1;
				interface->descriptors[2].flags = {.next = false, .write = true};

				interface->push_descriptor(0);
				io.notify_queue(0);

				wait();

				if (status != 0)
				{
					if (retry == max_retry_count - 1)
						return false;
					else
						continue;
				}

				std::memcpy(buffer, (void*)this->buffer->data, 512);
				buffer += 512;

				break;
			}
		}

		return true;
	}

	bool Block_device_interface::write_sectors(
		const uint8_t* buffer,
		size_t sector_begin,
		size_t sector_count
	)
	{
		wait();

		const auto queue_size = interface->size();
		if (queue_size < 3) return false;

		for (auto sector = sector_begin; sector < sector_begin + sector_count; sector++)
		{
			for (uint32_t retry = 0; retry < max_retry_count; retry++)
			{
				Block_request request = Block_request::write(sector);
				volatile uint8_t status;

				interface->descriptors[0].set_object(request);
				interface->descriptors[0].flags = {.next = true, .write = false};
				interface->descriptors[0].next = 1;

				std::memcpy((void*)this->buffer->data, buffer, 512);
				interface->descriptors[1].addr = reinterpret_cast<uintptr_t>(this->buffer->data);
				interface->descriptors[1].len = 512;
				interface->descriptors[1].flags = {.next = true, .write = true};
				interface->descriptors[1].next = 2;

				interface->descriptors[2].set_object(status);
				interface->descriptors[2].len = 1;
				interface->descriptors[2].flags = {.next = false, .write = true};

				interface->push_descriptor(0);
				io.notify_queue(0);

				wait();

				if (status != 0)
				{
					if (retry == max_retry_count - 1)
						return false;
					else
						continue;
				}

				buffer += 512;

				break;
			}
		}

		return true;
	}
}