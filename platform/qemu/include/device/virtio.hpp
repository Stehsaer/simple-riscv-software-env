#pragma once

#include "padding.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace device::virtio
{
	struct IO
	{
		static constexpr uint32_t expected_magic_value = 0x74726976;

		// 0x000
		volatile const uint32_t magic_value;
		volatile const uint32_t version;
		volatile const uint32_t device_id;
		volatile const uint32_t vendor_id;

		// 0x010
		volatile const uint32_t device_features;
		volatile uint32_t device_features_sel;  // 0x014

		MODULE_DEVICE_PADDING(0x018, 0x020, padding_1)

		// 0x020
		volatile uint32_t driver_features;
		volatile uint32_t driver_features_sel;  // 0x024

		MODULE_DEVICE_PADDING(0x028, 0x030, padding_2)

		// 0x030
		volatile uint32_t queue_sel;
		volatile uint32_t queue_num_max;
		volatile uint32_t queue_num;  // 0x038

		MODULE_DEVICE_PADDING(0x03C, 0x044, padding_3)

		volatile uint32_t queue_ready;  // 0x044

		MODULE_DEVICE_PADDING(0x048, 0x050, padding_4)

		// 0x050
		volatile uint32_t queue_notify;

		MODULE_DEVICE_PADDING(0x054, 0x060, padding_5)

		// 0x060
		volatile uint32_t interrupt_status;
		volatile uint32_t interrupt_ack;  // 0x064

		MODULE_DEVICE_PADDING(0x068, 0x070, padding_6)

		// 0x070
		volatile uint32_t status;

		MODULE_DEVICE_PADDING(0x074, 0x080, padding_7)

		// 0x080
		volatile uint32_t queue_desc_low;
		volatile uint32_t queue_desc_high;  // 0x084

		MODULE_DEVICE_PADDING(0x088, 0x090, padding_8)

		volatile uint32_t queue_driver_low;
		volatile uint32_t queue_driver_high;

		MODULE_DEVICE_PADDING(0x098, 0x0A0, padding_9)

		// 0x0A0
		volatile uint32_t queue_device_low;
		volatile uint32_t queue_device_high;  // 0x0A4

		MODULE_DEVICE_PADDING(0x0A8, 0x0FC, padding_10)

		// 0x0FC
		volatile const uint32_t config_generation;

		// 0x100
		volatile uint8_t config_space[0x1000 - 0x100];

		IO() = delete;
		~IO() = delete;
		IO(const IO&) = delete;
		IO(IO&&) = delete;

		// Check Magic Number
		bool check_magic_value() const { return magic_value == expected_magic_value; }

		// Select Queue Index as the current queue
		void select_queue(uint32_t queue_idx) { queue_sel = queue_idx; }

		// Inform the device of the size of the current queue
		void set_queue_size(uint32_t queue_size) { queue_num = queue_size; }

		// Notify the device about a queue
		void notify_queue(uint32_t queue_idx) { queue_notify = queue_idx; }

		// Confirm that the queue is ready
		void confirm_queue_ready() { queue_ready = 1; }

		// Check if the device is a valid Virtio block device
		bool is_block_device() const { return check_magic_value() && version == 2 && device_id == 2; }

		static IO& at(uint8_t no) { return *(IO*)(0x10001000 + no * 4096); }
	};

	struct Queue_descriptor
	{
		struct Flag
		{
			bool next     : 1 = false;
			bool write    : 1 = false;
			bool indirect : 1 = false;
		};

		uint64_t addr;
		uint32_t len;
		alignas(2) Flag flags;
		alignas(2) uint16_t next = 0;

		template <typename T>
		void set_object(T& obj)
		{
			addr = reinterpret_cast<uintptr_t>(&obj);
			len = sizeof(obj);
		}
	};

	struct Queue_avail_info
	{
		uint16_t flags;
		uint16_t idx;
		uint16_t ring[8];
	};

	struct Queue_used_info
	{
		volatile uint16_t flags;
		volatile uint16_t idx;
		volatile struct Used_elem
		{
			uint32_t id;
			uint32_t len;
		} ring[8];
	};

	enum class Request_type : uint32_t
	{
		Read = 0,
		Write = 1,
		Flush = 4,
		Get_device_id = 8,
	};

	struct Block_request
	{
		Request_type type;

	  private:

		uint32_t reserved [[maybe_unused]] = 0;

	  public:

		uint64_t sector;

		Block_request() :
			type(Request_type::Read),
			sector(0)
		{
		}

		Block_request(Request_type type, uint64_t sector) :
			type(type),
			sector(sector)
		{
		}

		static Block_request write(uint64_t sector) { return {Request_type::Write, sector}; }
		static Block_request read(uint64_t sector) { return {Request_type::Read, sector}; }
		static Block_request flush() { return {Request_type::Flush, 0}; }
	};

	struct Interface
	{
		std::vector<Queue_descriptor> descriptors;
		std::unique_ptr<Queue_avail_info> avail;
		std::unique_ptr<Queue_used_info> used;

		Interface(size_t queue_size) :
			descriptors(queue_size),
			avail(std::make_unique<Queue_avail_info>()),
			used(std::make_unique<Queue_used_info>())
		{
		}

		void assign_virtio(IO& virtio);
		void push_descriptor(uint16_t queue_idx) const;
		bool check_used_consumed() const;
		size_t size() const { return descriptors.size(); }
	};

	template <size_t Size>
	struct alignas(4096) Aligned_buffer
	{
		volatile uint8_t data[Size];
	};

	class Block_device_interface
	{
		IO& io;
		std::unique_ptr<Interface> interface;
		std::unique_ptr<Aligned_buffer<512>> buffer;

		static constexpr size_t default_queue_size = 4;

	  public:

		Block_device_interface(IO& virtio);

		// Get the count of sectors in device
		uint64_t get_device_size();

		// Read sectors from the device. Returns false if the operation fails.
		bool read_sectors(uint8_t* buffer, size_t sector_begin, size_t sector_count);

		// Write sectors to the device. Returns false if the operation fails.
		bool write_sectors(const uint8_t* buffer, size_t sector_begin, size_t sector_count);

		// Wait for not busy
		void wait() const
		{
			while (!interface->check_used_consumed())
			{
			}
		}
	};
}