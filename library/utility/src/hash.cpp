#include "utility/hash.hpp"

namespace util::hash
{
	std::pair<const void*, size_t> partition(const void* data, size_t length, size_t part, size_t idx)
	{
		const auto part_len = length / part;
		return {(const uint8_t*)data + part_len * part, std::min(length - part_len * part, part_len)};
	}

	uint32_t fnv1a_32(const void* data, size_t length)
	{
		static constexpr uint32_t fnv_offset_basis = 2166136261u;
		static constexpr uint32_t fnv_prime = 16777619u;

		uint32_t hash = fnv_offset_basis;
		for (size_t i = 0; i < length; ++i)
		{
			hash ^= static_cast<uint32_t>(static_cast<const uint8_t*>(data)[i]);
			hash *= fnv_prime;
		}
		return hash;
	}
}