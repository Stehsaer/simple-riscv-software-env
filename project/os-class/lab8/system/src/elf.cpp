#include "elf.hpp"
#include "allocator.hpp"
#include "env.hpp"
#include <cstring>

Program::Segment::~Segment()
{
	for (auto page : pages) allocator::deallocate_page(page);
}

std::optional<std::unique_ptr<Program>> Program::load(FILE* file)
{
	std::unique_ptr<Program> program = std::make_unique<Program>(Program());

	if (file == nullptr) return std::nullopt;

	// Read ELF Header
	elf::Elf32_header header;
	if (fread(&header, sizeof(header), 1, file) != 1) return std::nullopt;

	// Validate ELF Header
	if (!header.is_executable() || !header.is_executable() || !header.check_magic()) return std::nullopt;

	program->entry_point = header.e_entry;

	for (uint16_t i = 0; i < header.e_phnum; i++)
	{
		if (fseek(file, header.e_phoff + i * sizeof(elf::Elf32_program_header), SEEK_SET) != 0)
			return std::nullopt;
		elf::Elf32_program_header program_header;
		if (fread(&program_header, sizeof(program_header), 1, file) != 1) return std::nullopt;

		std::unique_ptr<Segment> segment = std::make_unique<Segment>();

		segment->r = program_header.p_flags.r();
		segment->w = program_header.p_flags.w();
		segment->x = program_header.p_flags.x();

		switch (program_header.p_type)
		{
		case elf::Segment_type::Load:
			segment->type = Segment::Type::Load;
			break;
		case elf::Segment_type::Tls:
			segment->type = Segment::Type::Thread_local;
			break;
		default:
			continue;
		}

		if (segment->type == Segment::Type::Thread_local)
		{
			segment->page_address = 0;

			// Allocate pages
			const size_t page_count = (program_header.p_memsz + 4095) / 4096;
			segment->pages.resize(page_count);
			for (auto& page : segment->pages) page = allocator::allocate_page();

			size_t current = 0;
			if (fseek(file, program_header.p_offset, SEEK_SET) != 0) return std::nullopt;

			// Read file content
			while (current < program_header.p_filesz)
			{
				size_t read_size = std::min<size_t>(4096, program_header.p_filesz - current);
				if (fread(
						reinterpret_cast<void*>(segment->pages[current / 4096] + current % 4096),
						read_size,
						1,
						file
					)
					!= 1)
					return std::nullopt;
				current += read_size;
			}

			// Zero-fill rest of the space
			while (current < program_header.p_memsz)
			{
				size_t zero_fill_size = std::min<size_t>(4096, program_header.p_memsz - current);
				memset(
					reinterpret_cast<void*>(segment->pages[current / 4096] + current % 4096),
					0,
					zero_fill_size
				);
				current += zero_fill_size;
			}
		}
		else
		{
			segment->page_address = program_header.p_vaddr & ~0xfff;  // Align to 4KiB

			// Allocate pages
			const size_t page_begin_id = segment->page_address >> 12;
			const size_t page_end_id = (program_header.p_vaddr + program_header.p_memsz + 4095) >> 12;
			const size_t page_count = page_end_id - page_begin_id;
			segment->pages.resize(page_count);
			for (auto& page : segment->pages) page = allocator::allocate_page();

			uintptr_t current = program_header.p_vaddr;
			if (fseek(file, program_header.p_offset, SEEK_SET) != 0) return std::nullopt;

			// Read file content
			while (current < program_header.p_vaddr + program_header.p_filesz)
			{
				size_t read_size
					= std::min<size_t>(4096, program_header.p_filesz - (current - program_header.p_vaddr));
				if (fread(
						reinterpret_cast<void*>(
							segment->pages[(current >> 12) - page_begin_id] + (current & 0xfff)
						),
						read_size,
						1,
						file
					)
					!= 1)
					return std::nullopt;
				current += read_size;
			}

			// Fill the rest of the space with zeros
			while (current < program_header.p_vaddr + program_header.p_memsz)
			{
				size_t zero_fill_size
					= std::min<size_t>(4096, program_header.p_memsz - (current - program_header.p_vaddr));
				memset(
					reinterpret_cast<void*>(
						segment->pages[(current >> 12) - page_begin_id] + (current & 0xfff)
					),
					0,
					zero_fill_size
				);
				current += zero_fill_size;
			}
		}

		program->segments.push_back(std::move(segment));
	}

	return program;
}
