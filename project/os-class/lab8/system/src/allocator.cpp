#include "allocator.hpp"
#include "env.hpp"

#include <cstring>
#include <random>

namespace allocator
{
#ifdef RVISA_F
#error "Floating point is not supported"
#endif

	struct Page_struct
	{
		alignas(4096) uint8_t data[4096];
	};

	using Bitmap_word = uint32_t;

	static constexpr size_t second_bitmap_ratio = 1024, total_page_count = 393216;
	static_assert(second_bitmap_ratio < std::numeric_limits<uint16_t>::max());
	static_assert(total_page_count % second_bitmap_ratio == 0);
	static_assert(second_bitmap_ratio % (sizeof(Bitmap_word) * 8) == 0);
	static_assert(total_page_count % (sizeof(Bitmap_word) * 8) == 0);

	static std::array<Page_struct, total_page_count> pages alignas(65536);
	static std::array<Bitmap_word, total_page_count / sizeof(Bitmap_word) / 8> bitmap;
	static std::array<uint16_t, total_page_count / second_bitmap_ratio> second_bitmap;
	static uint32_t allocated_count = 0;

	static uint32_t search_page = std::hash<std::string_view>()(std::string_view(__DATE__ __TIME__));

	uint32_t get_total_pages()
	{
		return total_page_count;
	}

	uint32_t get_allocated_pages()
	{
		return allocated_count;
	}

	uintptr_t allocate_page()
	{
		if (allocated_count >= total_page_count) KERNEL_ERROR("No free pages available.");

		const auto second_bitmap_index = [] -> size_t
		{
			while (true)
			{
				search_page = std::hash<std::string>()(std::to_string(search_page + 1));
				if (second_bitmap[search_page % second_bitmap.size()] < second_bitmap_ratio)
					return search_page % second_bitmap.size();
			}
		}();
		second_bitmap[second_bitmap_index]++;
		allocated_count++;

		const auto bitmap_search_start
			= bitmap.begin() + second_bitmap_index * second_bitmap_ratio / sizeof(Bitmap_word) / 8;
		const auto bitmap_search_end
			= bitmap.begin() + (second_bitmap_index + 1) * second_bitmap_ratio / sizeof(Bitmap_word) / 8;

		const auto find_bitmap_word = std::find_if(
			bitmap_search_start,
			bitmap_search_end,
			[](uint32_t val) { return val != 0xffffffff; }
		);

		KERNEL_ASSERT(
			find_bitmap_word != bitmap_search_end,
			"Logic error: No free pages available in the given range that's supposed to be free"
		);

		const auto find_bitmap_word_idx = std::distance(bitmap.begin(), find_bitmap_word);
		const auto available_bit = std::countr_one(*find_bitmap_word);
		KERNEL_ASSERT(available_bit < 32, "Logic error: No free bits available in the found bitmap word.");

		*find_bitmap_word |= (1 << available_bit);
		const auto bit_idx = find_bitmap_word_idx * sizeof(Bitmap_word) * 8 + available_bit;

		return uintptr_t(&pages[bit_idx]);
	}

	void deallocate_page(uintptr_t address)
	{
		const auto *begin = &(*pages.begin()), *end = &(*pages.end());

		KERNEL_ASSERT(
			address >= uintptr_t(begin) && address < uintptr_t(end) && ((address & 0xfff) == 0),
			"Invalid page pointer 0x%08x, valid range is 0x%08x <= address < 0x%08x, and must be 4KiB "
			"aligned",
			(uint32_t)address,
			(uint32_t)uintptr_t(begin),
			(uint32_t)uintptr_t(end)
		);

		const Page_struct* page_ptr = (Page_struct*)(address & (~(sizeof(Page_struct) - 1)));
		const auto idx = page_ptr - begin;

		bitmap[idx / (sizeof(Bitmap_word) * 8)] &= ~(1 << (idx % sizeof(Bitmap_word)));
		second_bitmap[idx / second_bitmap_ratio]--;
		allocated_count--;
	}

	void copy_page(uintptr_t from, uintptr_t to)
	{
		KERNEL_ASSERT(
			((from & 0xfff) == 0) && ((to & 0xfff) == 0),
			"Invalid page pointers, must be 4KiB aligned: from=0x%08x, to=0x%08x",
			(uint32_t)from,
			(uint32_t)to
		);

		auto* from_page = reinterpret_cast<Page_struct*>(from);
		auto* to_page = reinterpret_cast<Page_struct*>(to);

		std::memcpy(to_page->data, from_page->data, sizeof(Page_struct));
	}

	void zero_page(uintptr_t address)
	{
		KERNEL_ASSERT(
			(address & 0xfff) == 0,
			"Invalid page pointer, must be 4KiB aligned: address=0x%08x",
			(uint32_t)address
		);

		auto* page = reinterpret_cast<Page_struct*>(address);
		std::memset(page->data, 0, sizeof(Page_struct));
	}
}