#include "platform/periph/uart.hpp"

#include <cstring>

namespace periph
{
	void Uart::set_config(uint32_t divisor, Parity parity, Stopbits stopbits) volatile
	{
		union
		{
			struct
			{
				uint32_t _divisor  : 24;
				Parity _parity     : 3;
				Stopbits _stopbits : 1;
				int padding        : 4;
			} union_struct;
			uint32_t value;
		};

		union_struct._divisor = divisor;
		union_struct._parity = parity;
		union_struct._stopbits = stopbits;

		this->config = value;
	}
}