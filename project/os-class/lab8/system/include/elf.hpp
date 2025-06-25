#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace elf
{
	using Elf32_Addr = uint32_t;
	using Elf32_Half = uint16_t;
	using Elf32_Off = uint32_t;
	using Elf32_Word = uint32_t;

	struct Elf32_header
	{
		uint8_t e_ident[16];
		Elf32_Half e_type;
		Elf32_Half e_machine;
		Elf32_Word e_version;
		Elf32_Addr e_entry;
		Elf32_Off e_phoff;
		Elf32_Off e_shoff;
		Elf32_Word e_flags;
		Elf32_Half e_ehsize;
		Elf32_Half e_phentsize;
		Elf32_Half e_phnum;
		Elf32_Half e_shentsize;
		Elf32_Half e_shnum;
		Elf32_Half e_shstrndx;

		bool check_magic() const
		{
			return e_ident[0] == 0x7F && e_ident[1] == 'E' && e_ident[2] == 'L' && e_ident[3] == 'F';
		}
		bool is_riscv() const { return e_machine == 243; }
		bool is_executable() const { return e_type == 2; }
	};

	enum class Segment_type : Elf32_Word
	{
		Null = 0,
		Load = 1,
		Dynamic = 2,
		Interp = 3,
		Note = 4,
		Shlib = 5,
		Phdr = 6,
		Tls = 7,
	};

	struct Segment_flag
	{
	  private:

		Elf32_Word p_flags;

	  public:

		bool r() const { return p_flags & 0x4; }
		bool w() const { return p_flags & 0x2; }
		bool x() const { return p_flags & 0x1; }
	};

	struct Elf32_program_header
	{
		Segment_type p_type;
		Elf32_Off p_offset;
		Elf32_Addr p_vaddr;
		Elf32_Addr p_paddr;
		Elf32_Word p_filesz;
		Elf32_Word p_memsz;
		Segment_flag p_flags;
		Elf32_Word p_align;
	};

	struct Elf32_section_header
	{
		Elf32_Word sh_name;
		Elf32_Word sh_type;
		Elf32_Word sh_flags;
		Elf32_Addr sh_addr;
		Elf32_Off sh_offset;
		Elf32_Word sh_size;
		Elf32_Word sh_link;
		Elf32_Word sh_info;
		Elf32_Word sh_addralign;
		Elf32_Word sh_entsize;
	};
}

struct Program
{
	struct Segment
	{
		Segment(const Segment&) = delete;
		Segment(Segment&&) = delete;
		Segment& operator=(const Segment&) = delete;
		Segment& operator=(Segment&&) = delete;

		Segment() = default;
		~Segment();

		enum class Type : uint8_t
		{
			Invalid,
			Load,
			Thread_local
		} type = Type::Invalid;

		bool r = false, w = false, x = false;

		uintptr_t page_address = 0;    // (Virtual Address)
		std::vector<uintptr_t> pages;  // (4KiB Aligned Physical Addresses)
	};

	Program(const Program&) = delete;
	Program(Program&&) = default;
	Program& operator=(const Program&) = delete;
	Program& operator=(Program&&) = default;

	std::vector<std::unique_ptr<Segment>> segments;
	uintptr_t entry_point = 0;  // (Virtual Address)

	static std::optional<std::unique_ptr<Program>> load(FILE* file);

  private:

	Program() = default;
};