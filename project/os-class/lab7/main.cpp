#include <algorithm>
#include <cassert>
#include <cmath>
#include <print>
#include <random>
#include <vector>

#include <filesystem.hpp>
#include <platform.hpp>

static void init_environment()
{
	auto& uart = platform::uart;
	filesystem::fs.mount_device("uart:/", std::make_unique<filesystem::driver::Serial>(uart));
	freopen("uart:/", "w", stdout);
}

struct Page_struct
{
	alignas(4096) uint8_t data[4096];
};

struct Page_manager
{
	static constexpr size_t second_bitmap_ratio = 64;
	static_assert(second_bitmap_ratio < std::numeric_limits<uint16_t>::max());

	size_t page_count;
	std::vector<Page_struct> pages;
	std::vector<bool> bitmap;
	std::vector<uint16_t> second_bitmap;

  public:

	Page_manager(size_t count) :
		page_count(count),
		pages(count),
		bitmap(count, false),
		second_bitmap(
			(count % second_bitmap_ratio != 0) ? (count / second_bitmap_ratio + 1)
											   : (count / second_bitmap_ratio),
			0
		)
	{
	}

	uintptr_t allocate_page()
	{
		const auto find_second_bitmap = std::ranges::find_if(
			second_bitmap,
			[](auto val) { return val < Page_manager::second_bitmap_ratio; }
		);
		if (find_second_bitmap == second_bitmap.end()) throw std::runtime_error("No free pages available.");

		(*find_second_bitmap)++;
		const auto second_bitmap_idx = std::distance(second_bitmap.begin(), find_second_bitmap);

		const auto find_first_bitmap = std::find(
			bitmap.begin() + second_bitmap_idx * second_bitmap_ratio,
			bitmap.begin() + (second_bitmap_idx + 1) * second_bitmap_ratio,
			false
		);
		if (find_first_bitmap == bitmap.begin() + (second_bitmap_idx + 1) * second_bitmap_ratio)
			throw std::runtime_error("No free pages available in the second bitmap.");

		*find_first_bitmap = true;
		const auto first_bitmap_idx = std::distance(bitmap.begin(), find_first_bitmap);
		return uintptr_t(&pages[first_bitmap_idx]);
	}

	void deallocate_page(uintptr_t address)
	{
		const auto *begin = &(*pages.begin()), *end = &(*pages.end());
		if (address < uintptr_t(begin) || address >= uintptr_t(end))
			throw std::runtime_error("Invalid page pointer.");

		const Page_struct* page_ptr = (Page_struct*)(address & (~(sizeof(Page_struct) - 1)));
		const auto idx = page_ptr - begin;

		bitmap[idx] = false;
		second_bitmap[idx / second_bitmap_ratio]--;
	}
};

static void set_satp(bool paging_enabled, uintptr_t page)
{
	if ((page & 0xfff) != 0) throw std::runtime_error("Page table address must be 4KiB aligned!");

	const uint32_t satp_value = ((paging_enabled ? 0x1 : 0x0) << 31) | (page >> 12);
	asm volatile("csrw satp, %0" : : "r"(satp_value));
}

static void set_mepc(uintptr_t epc)
{
	asm volatile("csrw mepc, %0" : : "r"(epc));
}

static void set_mtvec(uintptr_t handler)
{
	asm volatile("csrw mtvec, %0" : : "r"(handler & ~0x3));
}

void setup_pmp(void)
{
	// Set up a PMP to permit access to all of memory.
	// Ignore the illegal-instruction trap if PMPs aren't supported.
	const uintptr_t pmpc = 0b11111;
	asm volatile("la t0, 1f\n\t"
				 "csrrw t0, mtvec, t0\n\t"
				 "csrw pmpaddr0, %1\n\t"
				 "csrw pmpcfg0, %0\n\t"
				 ".align 2\n\t"
				 "1: csrw mtvec, t0"
				 :
				 : "r"(pmpc), "r"(-1UL)
				 : "t0");
}

class Page_table_entry
{
	uint32_t value;

  public:

	Page_table_entry& operator=(const Page_table_entry&) = default;
	Page_table_entry& operator=(Page_table_entry&&) = default;
	Page_table_entry(const Page_table_entry&) = default;
	Page_table_entry(Page_table_entry&&) = default;

	struct Priviledge
	{
		bool v : 1 = false, r : 1 = false, w : 1 = false, x : 1 = false, u : 1 = false, g : 1 = false,
				 a : 1 = false, d : 1 = false;
	};
	static_assert(sizeof(Priviledge) == 1, "Priviledge must be 1 byte in size!");

	Page_table_entry() :
		value(0)
	{
	}

	Page_table_entry(uintptr_t address, Priviledge priv)
	{
		if ((address & 0xfff) != 0) throw std::runtime_error("Page table address must be 4KiB aligned!");

		const uint8_t priv_int = *reinterpret_cast<uint8_t*>(&priv);

		value = (address >> 2) | (priv_int << 0);
	}

	Priviledge get_priv() const { return *reinterpret_cast<const Priviledge*>(&value); }
	uintptr_t get_address() const { return ((value >> 10) & 0xfffff) << 12; }
};

class Page_table
{
	alignas(4096) std::array<Page_table_entry, 1024> table;

  public:

	Page_table() = default;

	uintptr_t get_base_address() const { return uintptr_t(table.data()); }
	void set_as_primary_table() const { set_satp(true, get_base_address()); }

	Page_table_entry& operator[](size_t offset)
	{
		if (offset >= 1024) throw std::logic_error("Offset must be < 1024, logic error!");
		return table[offset];
	}

	const Page_table_entry& operator[](size_t offset) const
	{
		if (offset >= 1024) throw std::logic_error("Offset must be < 1024, logic error!");
		return table[offset];
	}
};

class Page_table_manager
{
	Page_manager page_manager{1024};
	Page_table* primary_page_table = nullptr;

	void check_address_valid(uintptr_t virtual_address)
	{
		if ((virtual_address & 0xfff) != 0) throw std::runtime_error("Address must be 4KiB aligned!");
	}

	Page_table_entry& find_or_allocate_entry(uintptr_t virtual_address)
	{
		check_address_valid(virtual_address);

		if (primary_page_table == nullptr)
		{
			primary_page_table = (Page_table*)page_manager.allocate_page();
			std::construct_at(primary_page_table);
		}

		auto& first_page_table_item = (*primary_page_table)[virtual_address >> 22];

		if (!first_page_table_item.get_priv().v)
		{
			const auto new_allocated = page_manager.allocate_page();
			auto* new_ptr = (Page_table*)new_allocated;

			first_page_table_item = {new_allocated, {.v = true}};
			std::construct_at(new_ptr);

			return (*new_ptr)[(virtual_address >> 12) & 0b11111'11111];
		}

		auto* page_table_ptr = (Page_table*)first_page_table_item.get_address();
		return (*page_table_ptr)[(virtual_address >> 12) & 0b11111'11111];
	}

  public:

	Page_table_manager(const Page_table_manager&) = delete;
	Page_table_manager(Page_table_manager&&) = delete;
	Page_table_manager() = default;

	uintptr_t allocate(uintptr_t virtual_address, bool r, bool w, bool x)
	{
		auto& page_item = find_or_allocate_entry(virtual_address);

		const auto allocated_address = page_manager.allocate_page();
		page_item = {
			allocated_address,
			{.v = true, .r = r, .w = w, .x = x}
		};

		return allocated_address;
	}

	void deallocate(uintptr_t virtual_address)
	{
		auto& page_item = find_or_allocate_entry(virtual_address);

		const auto original_address = page_item.get_address();
		page_manager.deallocate_page(original_address);

		page_item = {0, {}};
	}

	void assign(uintptr_t virtual_address, uintptr_t physical_address, bool r, bool w, bool x)
	{
		auto& page_item = find_or_allocate_entry(virtual_address);
		check_address_valid(physical_address);

		page_item = {
			physical_address,
			{.v = true, .r = r, .w = w, .x = x}
		};
	}

	void deassign(uintptr_t virtual_address)
	{
		auto& page_item = find_or_allocate_entry(virtual_address);
		page_item = {0, {}};
	}

	void set_as_primary() const
	{
		if (primary_page_table == nullptr)
			throw std::logic_error("Can't set an empty page table as primary, logic error!");

		primary_page_table->set_as_primary_table();
	}

	uintptr_t convert(uintptr_t virtual_address)
	{
		return find_or_allocate_entry(virtual_address).get_address();
	}
};

extern "C"
{
	extern uint8_t code_start, code_end;
}

[[noreturn]] static void trap_handler()
{
	static std::string_view trap_name[20] = {
		"Instruction address misaligned",
		"Instruction access fault",
		"Illegal instruction",
		"Breakpoint",
		"Load address misaligned",
		"Load access fault",
		"Store/AMO address misaligned",
		"Store/AMO access fault",
		"Environment call from U-mode",
		"Environment call from S-mode",
		"Reserved",
		"Environment call from M-mode",
		"Instruction page fault",
		"Load page fault",
		"Reserved",
		"Store/AMO page fault",
		"Reserved",
		"Reserved",
		"Software check",
		"Hardware error",
	};

	uint32_t mcause;
	asm volatile("csrr %0, mcause" : "=r"(mcause));

	std::println("Trap handler invoked!");

	if (mcause < 20)
	{
		std::println("Trap: {}", trap_name[mcause]);
	}
	else
	{
		std::println("Trap: Unknown trap code 0x{:08x}", mcause);
	}

	uint32_t mepc, mtval;
	asm volatile("csrr %0, mepc" : "=r"(mepc));
	asm volatile("csrr %0, mtval" : "=r"(mtval));

	std::println("mepc = 0x{:08x}, mtval = 0x{:08x}", mepc, mtval);

	while (true);
}

int main()
{
	init_environment();
	setup_pmp();
	std::println("OS Class Lab 7");

	Page_table_manager page_table;

	try
	{
		const uintptr_t code_page = page_table.allocate(0x10000000, true, false, true),
						data_page = page_table.allocate(0x20000000, true, true, false);
		page_table.assign(0x30000000, uintptr_t(&platform::uart), true, true, false);
		page_table.set_as_primary();

		const std::string_view test_string = "Hello, OS Class, 23336160!\n";
		std::ranges::copy(test_string, reinterpret_cast<char*>(data_page));

		std::fill(reinterpret_cast<char*>(code_page), reinterpret_cast<char*>(code_page + 4096), 0);
		std::copy(
			reinterpret_cast<const char*>(&code_start),
			reinterpret_cast<const char*>(&code_end),
			reinterpret_cast<char*>(code_page)
		);

		std::println("code_page = 0x{:08x}", code_page);
		std::println("data_page = 0x{:08x}", data_page);
		std::println("converted code_page = 0x{:08x}", page_table.convert(0x10000000));
		std::println("converted data_page = 0x{:08x}", page_table.convert(0x20000000));

		set_mtvec(uintptr_t(&trap_handler));
		set_mepc(0x10000000);

		uint32_t mstatus;
		asm volatile("csrr %0, mstatus" : "=r"(mstatus));
		mstatus &= ~(0b11 << 11);  // Clear MPP bits
		mstatus |= (0b01 << 11);   // Set MPP to 01 (Supervisor mode)
		asm volatile("csrw mstatus, %0" : : "r"(mstatus));

		asm volatile("fence.i");
		asm volatile("sfence.vma zero, zero");  // Flush TLB
	}
	catch (const std::exception& e)
	{
		std::println("Exception: {}", e.what());
		return 1;
	}

	asm volatile("mret");
}
