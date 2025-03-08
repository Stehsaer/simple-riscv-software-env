#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace util::hash
{
	std::pair<const void*, size_t> partition(const void* data, size_t length, size_t part, size_t idx);
	uint32_t fnv1a_32(const void* data, size_t length);

	template <typename T>
	using Hash_func = T(const void* data, size_t length);

	template <typename T, size_t Len>
	std::array<T, Len> gen_hash(const void* data, size_t len, Hash_func<T> func)
	{
		std::array<T, Len> arr;

		for (size_t i = 0; i < Len; i++)
		{
			const auto [par_data, par_len] = partition(data, len, Len, i);
			arr[i] = func(par_data, par_len);
		}

		return arr;
	}
}