#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>

#ifndef __cplusplus
#error "Don't include from C!"
#endif

#include "periph-base.hpp"

namespace periph
{
	class Uart
	{

	  public:

		alignas(4) char tx;
		alignas(4) char rx;

	  private:

		alignas(4) uint32_t config;

	  public:

		struct Status
		{
			bool rx_a : 1;  // Rx available
			bool tx_a : 1;  // Tx available
			bool err  : 1;  // Parity Error
		};

		alignas(4) Status status;

		enum class Stopbits
		{
			Bit1 = 0,
			Bit2 = 1
		};

		enum class Parity
		{
			None = 0b000,
			Odd = 0b001,
			Even = 0b010,
			Always1 = 0b100,
			Always0 = 0b010
		};

		void set_config(uint32_t divisor, Parity parity, Stopbits) volatile;
	};
}