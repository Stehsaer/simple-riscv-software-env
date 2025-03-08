#include "utility/hex-reader.hpp"

#include <cstring>

namespace util::intel_hex
{

	static uint8_t hex_to_byte(char c)
	{
		if (c >= '0' && c <= '9')
		{
			return c - '0';
		}
		if (c >= 'A' && c <= 'F')
		{
			return c - 'A' + 10;
		}
		if (c >= 'a' && c <= 'f')
		{
			return c - 'a' + 10;
		}
		return 0;
	}

	static uint8_t hex_to_byte(const char* c)
	{
		return (hex_to_byte(c[0]) << 4) | hex_to_byte(c[1]);
	}

	Payload Reader::read(const char* str)
	{
		Payload payload;

		/* CHECK LENGTH */

		const auto len = strlen(str);

		// invalid length
		if (len < 11)
		{
			payload.type = Payload_type::Error;
			payload.error_type = Error_type::Invalid_length;
			return payload;
		}

		// invalid start
		if (str[0] != ':')
		{
			payload.type = Payload_type::Error;
			payload.error_type = Error_type::Invalid_char;
			return payload;
		}

		const auto data_length = hex_to_byte(str + 1);

		// invalid length
		if (len != 11 + data_length * 2)
		{
			payload.type = Payload_type::Error;
			payload.error_type = Error_type::Invalid_length;
			return payload;
		}

		const auto type = hex_to_byte(str + 7);
		const uint16_t address = (hex_to_byte(str + 3) << 8) | hex_to_byte(str + 5);
		const auto checksum = hex_to_byte(str + 9 + data_length * 2);

		/* VALIDATE CHECKSUM */

		int sum = data_length;
		sum += type;
		sum += (address >> 8) & 0xFF;
		sum += address & 0xFF;

		std::array<uint8_t, 256> data;
		for (uint8_t i = 0; i < data_length; ++i)
		{
			data[i] = hex_to_byte(str + 9 + i * 2);
			sum += data[i];
		}

		uint8_t sum8 = sum & 0xFF;
		sum8 = ~sum8 + 1;

		if (sum8 != checksum)
		{
			payload.type = Payload_type::Error;
			payload.error_type = Error_type::Invalid_checksum;
			return payload;
		}

		/* PROCESSING */

		switch (type)
		{
		case 0:
		{
			payload.type = Payload_type::Data;
			payload.error_type = Error_type::No_error;
			payload.address = base_address + address;
			payload.data_length = data_length;
			memcpy(payload.data.data(), data.data(), data_length);

			return payload;
		}
		case 1:  // end of file
		{
			payload.type = Payload_type::End_of_file;
			payload.error_type = Error_type::No_error;
			return payload;
		}
		case 2:  // extended segment address
		{
			if (data_length != 2)
			{
				payload.type = Payload_type::Error;
				payload.error_type = Error_type::Invalid_length;
				return payload;
			}

			base_address = (data[0] << 8) | data[1];
			base_address *= 16;

			payload.type = Payload_type::Empty;
			payload.error_type = Error_type::No_error;
			return payload;
		}
		case 4:
		{
			if (data_length != 2)
			{
				payload.type = Payload_type::Error;
				payload.error_type = Error_type::Invalid_length;
				return payload;
			}

			base_address = (data[0] << 8) | data[1];
			base_address <<= 16;

			payload.type = Payload_type::Empty;
			payload.error_type = Error_type::No_error;
			return payload;
		}
		case 5:
		{
			if (data_length != 4)
			{
				payload.type = Payload_type::Error;
				payload.error_type = Error_type::Invalid_length;
				return payload;
			}

			payload.type = Payload_type::Start_linear_address;
			payload.error_type = Error_type::No_error;
			payload.address = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
			return payload;
		}
		case 3:
		{
			payload.type = Payload_type::Empty;
			payload.error_type = Error_type::No_error;
			return payload;
		}
		default:
		{
			payload.type = Payload_type::Error;
			payload.error_type = Error_type::Invalid_type;
			return payload;
		}
		}
	}
}