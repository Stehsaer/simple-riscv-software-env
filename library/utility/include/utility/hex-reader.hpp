#pragma once

#include <array>
#include <cstdint>

namespace util::intel_hex
{
	enum class Payload_type
	{
		Data,
		End_of_file,
		Start_linear_address,
		Empty,
		Error
	};

	enum class Error_type
	{
		No_error,
		Invalid_length,
		Invalid_char,
		Invalid_checksum,
		Invalid_type
	};

	struct Payload
	{
		Payload_type type;

		Error_type error_type;

		uint32_t address = 0;
		uint8_t data_length = 0;
		std::array<uint8_t, 256> data = {0};
	};

	class Reader
	{
		uint32_t base_address = 0;

	  public:

		Reader() = default;

		Payload read(const char* str);
	};
}