#include "module/file/driver/uart.hpp"
#include "module/file/interface.hpp"
#include "module/platform/kintex7.hpp"

#include <string_view>

struct Screen_char
{
	char c = ' ';
	uint8_t fg_r = 0, fg_g = 0, fg_b = 0;
	uint8_t bg_r = 0, bg_g = 0, bg_b = 0;
};

class Screen
{
	Screen_char buffer[80][25];

  public:

	void print()
	{
		// Move to start of screen
		printf("\033[H");

		// Hide cursor
		printf("\033[?25l");

		for (int y = 0; y < 25; y++)
		{
			for (auto& x : buffer)
			{
				auto& c = x[y];

				// Set foreground color
				printf("\033[38;2;%d;%d;%dm", c.fg_r, c.fg_g, c.fg_b);

				// Set background color
				printf("\033[48;2;%d;%d;%dm", c.bg_r, c.bg_g, c.bg_b);

				// Print character
				putchar(c.c);
			}

			// Move to next line
			putchar('\n');
		}
	}

	Screen_char& operator[](size_t x, size_t y) { return buffer[x][y]; }
};

int main()
{
	auto& uart = platform::kintex7::uart;
	uart.configure(104, device::kintex7::Uart::Parity::Odd, device::kintex7::Uart::Stopbits::Bit1);
	file::fs.mount_device("uart:/", std::make_unique<file::driver::kintex7::Uart_driver>(uart));

	freopen("uart:/", "w", stdout);

	Screen screen;

	constexpr size_t width = 80, height = 25;
	size_t x = 2, y = 0;
	size_t idx = 0;
	const std::string_view sv = "1357924680";

	bool down = true, right = true;

	while (true)
	{
		auto& sc = screen[x, y];
		sc.c = sv[idx % sv.length()];
		sc.fg_r = rand() % 256;
		sc.fg_g = rand() % 256;
		sc.fg_b = rand() % 256;
		sc.bg_r = rand() % 256;
		sc.bg_g = rand() % 256;
		sc.bg_b = rand() % 256;

		screen.print();

		bool top_edge = y == 0;
		bool bottom_edge = y == height - 1;
		bool left_edge = x == 0;
		bool right_edge = x == width - 1;

		uint8_t edge_mask = (top_edge << 3) | (bottom_edge << 2) | (left_edge << 1) | (right_edge << 0);

		switch (edge_mask)
		{
		case 0b0001:  // Right edge
			right = false;
			break;
		case 0b0010:  // Left edge
			right = true;
			break;
		case 0b1000:  // Top edge
			down = true;
			break;
		case 0b0100:  // Bottom edge
			down = false;
			break;
		case 0b0001 | 0b1000:  // Right top corner
			right = false;
			down = true;
			break;
		case 0b0001 | 0b0100:  // Right bottom corner
			right = false;
			down = false;
			break;
		case 0b0010 | 0b1000:  // Left top corner
			right = true;
			down = true;
			break;
		case 0b0010 | 0b0100:  // Left bottom corner
			right = true;
			down = false;
			break;
		default:
			break;
		}

		x += right ? 1 : -1;
		y += down ? 1 : -1;
		idx++;
	}

	return 0;
}