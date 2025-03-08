#include <stdio.h>
#include <stdlib.h>
#include <time.h>

extern const char* const* const frames[];
extern const char* colors[256];
extern const int width, height;

void init_uart();

void goto_begin()
{
	printf("\033[1;1H\033[?25l");
}

void reset_screen()
{
	printf("\033[0m");
}

void check_frame_data()
{
	printf("Checking Frame data...\n");

	int idx = 0;
	while (1)
	{
		if (frames[idx] == NULL) break;

		const char* const* frame = frames[idx];

		for (int y = 0; y < height; y++)
		{
			const char* str = frame[y];

			for (int x = 0; x < width; x++)
				if (colors[str[x]] == NULL)
				{
					printf("Found excepted character %d at (%d, %d), address %p\n", str[x], y, x, str + x);
					exit(1);
				}
		}

		idx++;
	}
}

void init_color()
{
	colors[','] = "\033[48;5;17m";  /* Blue background */
	colors['.'] = "\033[48;5;231m"; /* White stars */
	colors['\''] = "\033[48;5;16m"; /* Black border */
	colors['@'] = "\033[48;5;230m"; /* Tan poptart */
	colors['$'] = "\033[48;5;175m"; /* Pink poptart */
	colors['-'] = "\033[48;5;162m"; /* Red poptart */
	colors['>'] = "\033[48;5;196m"; /* Red rainbow */
	colors['&'] = "\033[48;5;214m"; /* Orange rainbow */
	colors['+'] = "\033[48;5;226m"; /* Yellow Rainbow */
	colors['#'] = "\033[48;5;118m"; /* Green rainbow */
	colors['='] = "\033[48;5;33m";  /* Light blue rainbow */
	colors[';'] = "\033[48;5;19m";  /* Dark blue rainbow */
	colors['*'] = "\033[48;5;240m"; /* Gray cat face */
	colors['%'] = "\033[48;5;175m"; /* Pink cheeks */
}

char last = '\0';

clock_t frametime[10] = {0};

void draw_frame(const char* const frame[], int width, int height)
{
	goto_begin();
	last = '\0';

	clock_t avg_frametime = 0;

	for (int i = 0; i < 9; i++)
	{
		avg_frametime += frametime[i + 1] - frametime[i];
		frametime[i] = frametime[i + 1];
	}
	avg_frametime /= 9;
	frametime[9] = clock();

	float framerate = (float)CLOCKS_PER_SEC / avg_frametime;

	for (int y = 0; y < height; y++)
	{
		const char* str = frame[y];

		for (int x = 0; x < width; x++)
		{
			const char* color = colors[frame[y][x]];

			if (last != frame[y][x])
				printf("%s  ", color);
			else
				printf("  ");

			last = frame[y][x];
		}
		putc('\n', stdout);
	}

	printf("\033[0m%.2f FPS\n", framerate);
}

int main()
{
	init_uart();
	reset_screen();
	init_color();
	check_frame_data();

	int idx = 0;

	while (1)
	{
		if (frames[idx] == NULL) idx = 0;

		draw_frame(frames[idx], width, height);

		idx++;
	}

	reset_screen();
	return 0;
}