#include <algorithm>
#include <cmath>
#include <cstdint>

#include <format>
#include <iostream>
#include <print>
#include <random>
#include <span>
#include <string>
#include <vector>

#include "os/env.hpp"
#include "platform-v1/periph.hpp"
#include "platform-v1/sd.hpp"

#include <jpeglib.h>

#include <stb_image.h>

const char* fresult_string[]
	= {[FR_OK] = "FR_OK",
	   [FR_DISK_ERR] = "FR_DISK_ERR",
	   [FR_INT_ERR] = "FR_INT_ERR",
	   [FR_NOT_READY] = "FR_NOT_READY",
	   [FR_NO_FILE] = "FR_NO_FILE",
	   [FR_NO_PATH] = "FR_NO_PATH",
	   [FR_INVALID_NAME] = "FR_INVALID_NAME",
	   [FR_DENIED] = "FR_DENIED",
	   [FR_EXIST] = "FR_EXIST",
	   [FR_INVALID_OBJECT] = "FR_INVALID_OBJECT",
	   [FR_WRITE_PROTECTED] = "FR_WRITE_PROTECTED",
	   [FR_INVALID_DRIVE] = "FR_INVALID_DRIVE",
	   [FR_NOT_ENABLED] = "FR_NOT_ENABLED",
	   [FR_NO_FILESYSTEM] = "FR_NO_FILESYSTEM",
	   [FR_MKFS_ABORTED] = "FR_MKFS_ABORTED",
	   [FR_TIMEOUT] = "FR_TIMEOUT",
	   [FR_LOCKED] = "FR_LOCKED",
	   [FR_NOT_ENOUGH_CORE] = "FR_NOT_ENOUGH_CORE",
	   [FR_TOO_MANY_OPEN_FILES] = "FR_TOO_MANY_OPEN_FILES",
	   [FR_INVALID_PARAMETER] = "FR_INVALID_PARAMETER"};

class Image
{
  public:

	struct Color
	{
		uint8_t r = 0, g = 0, b = 0;
	};

	const Color* data() const { return pixels.data(); }
	Color* data() { return pixels.data(); }

	Image(size_t width, size_t height) :
		width(width),
		height(height)
	{
		pixels.resize(width * height);
	}

	const size_t width, height;

	Color& operator[](size_t x, size_t y) { return pixels[y * width + x]; }
	const Color& operator[](size_t x, size_t y) const { return pixels[y * width + x]; }

  private:

	std::vector<Color> pixels;
};

Image decode_jpeg(std::span<const uint8_t> jpeg_data)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	jpeg_mem_src(&cinfo, jpeg_data.data(), jpeg_data.size());
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	Image img(cinfo.output_width, cinfo.output_height);

	while (cinfo.output_scanline < cinfo.output_height)
	{
		JSAMPROW row_pointer = reinterpret_cast<JSAMPROW>(img.data() + cinfo.output_scanline * cinfo.output_width);
		jpeg_read_scanlines(&cinfo, &row_pointer, 1);
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	return img;
}

class Uart_mount_manager
{
  public:

	FILE *rd, *wr;

	Uart_mount_manager()
	{
		const auto mount_uart_result = os::fs.mount_device("uart1:", platform_v1::uart1.get_device());
		if (mount_uart_result != 0) exit(1);

		rd = freopen("uart1:/", "r", stdin);
		if (rd == nullptr) exit(1);

		wr = freopen("uart1:/", "w", stdout);
		if (wr == nullptr) exit(1);
	}

	~Uart_mount_manager()
	{
		fclose(rd);
		fclose(wr);
		os::fs.unmount_device("uart1:");
	}
};

int main()
{
	Uart_mount_manager uart_mount_manager;

	platform_v1::sd::set_speed(400);
	driver::sd::mount_media_handler();

	for (int i = 0; i < 10; i++)
	{
		driver::sd::reset_sd_library();
		if (driver::fat32::mount_disk().has_value())
		{
			printf("Disk mount failed. Retrying %d/10\n", i + 1);

			if (i != 9)
			{
				sleep(2);
				continue;
			}
			else
			{
				printf("Disk moutn failed. Wont retry. Terminating\n");
				return 1;
			}
		}
		break;
	}

	const auto mount_result = os::fs.mount_device("sd:", std::make_unique<driver::fat32::Device>());

	if (mount_result != 0)
	{
		printf("Failed to mount to fs\n");
		return 3;
	}

	platform_v1::sd::set_speed(25000);

	auto img = Image(2560, 1440);

	for (int y = 0; y < img.height; ++y)
	{
		for (int x = 0; x < img.width; ++x)
		{
			img[x, y].r = static_cast<uint8_t>(std::clamp<uint32_t>(x * 255 / img.width, 0, 255));
			img[x, y].g = static_cast<uint8_t>(std::clamp<uint32_t>(y * 255 / img.height, 0, 255));
			img[x, y].b = 0;
		}
	}

	std::println("Encoding JPG...");
	const auto encode_start_time = clock();
	std::vector<uint8_t> encoded_data;
	{
		int quality = 90;

		struct jpeg_compress_struct cinfo;
		struct jpeg_error_mgr jerr;

		cinfo.err = jpeg_std_error(&jerr);
		jpeg_create_compress(&cinfo);

		uint8_t* out_buffer = nullptr;
		size_t out_size = 0;

		jpeg_mem_dest(&cinfo, &out_buffer, &out_size);

		cinfo.image_width = img.width;
		cinfo.image_height = img.height;
		cinfo.input_components = 3;
		cinfo.in_color_space = JCS_RGB;

		jpeg_set_defaults(&cinfo);
		jpeg_set_quality(&cinfo, quality, TRUE);

		jpeg_start_compress(&cinfo, TRUE);

		JSAMPROW row_pointer[1];

		while (cinfo.next_scanline < cinfo.image_height)
		{
			row_pointer[0] = (JSAMPROW)&img[0, cinfo.next_scanline];
			jpeg_write_scanlines(&cinfo, row_pointer, 1);
		}

		jpeg_finish_compress(&cinfo);
		jpeg_destroy_compress(&cinfo);

		encoded_data.assign(out_buffer, out_buffer + out_size);

		free(out_buffer);
		std::println("Encoding Done in {} ms", (clock() - encode_start_time) * 1000 / CLOCKS_PER_SEC);
		std::println("Output JPG size: {} bytes", encoded_data.size());
	}

	std::println("Writing to disk...");
	auto* file = fopen("sd:/out.jpg", "w+b");
	if (file == nullptr)
	{
		printf("Failed to open file\n");
		return 2;
	}

	const size_t write_size = encoded_data.size();
	const auto actual_write = fwrite(encoded_data.data(), 1, write_size, file);

	if (actual_write != write_size)
	{
		printf(
			"Failed to write file, expected %d bytes, written %d bytes; errno=%d, FRESULT=%s\n",
			write_size,
			actual_write,
			errno,
			fresult_string[driver::fat32::last_failure]
		);
	}

	fclose(file);
	std::println("Writing done");

	driver::fat32::unmount_disk();

	return 0;
}