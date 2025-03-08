#pragma once

#include <cstddef>
#include <cstdint>

#include "../address-config.hpp"

#ifndef __cplusplus
#error "Don't include from C!"
#endif

namespace periph
{
	template <typename T>
		requires(sizeof(T) < 32)
	constexpr volatile T& get_block(size_t block_index) noexcept
	{
		return *static_cast<volatile T*>(static_cast<void*>(static_cast<uint8_t*>(platform_config.periph_start) + 256 * block_index));
	}
}