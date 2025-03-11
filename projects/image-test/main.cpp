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

#define STB_IMAGE_IMPLEMENTATION
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

	// Reading image
	std::println("Reading image...");
	const auto read_start_time = clock();
	std::vector<uint8_t> jpeg_data;
	{
		auto* file = fopen("sd:/test.png", "r");
		if (file == nullptr)
		{
			printf("Failed to open file\n");
			return 1;
		}

		fseek(file, 0, SEEK_END);
		const size_t file_size = ftell(file);
		fseek(file, 0, SEEK_SET);

		jpeg_data.resize(file_size);
		const auto read = fread(jpeg_data.data(), 1, file_size, file);

		if (read != file_size)
		{
			printf("Failed to read file\n");
			return 1;
		}

		fclose(file);

		std::println("Reading Done in {} ms", (clock() - read_start_time) * 1000 / CLOCKS_PER_SEC);
		std::println("Input JPG size: {} bytes", jpeg_data.size());
	}

	std::println("Decoding image...");
	std::unique_ptr<Image> image;
	const auto decode_start_time = clock();
	{
		// STBI
		int width, height, channels;
		const auto stbi_data = stbi_load_from_memory(jpeg_data.data(), jpeg_data.size(), &width, &height, &channels, 3);

		if (stbi_data == nullptr)
		{
			printf("Failed to decode image\n");
			return 1;
		}

		image = std::make_unique<Image>(width, height);

		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				const auto* pixel = stbi_data + (y * width + x) * 3;
				(*image)[x, y] = {pixel[0], pixel[1], pixel[2]};
			}
		}

		stbi_image_free(stbi_data);

		std::println("Decoding Done in {} ms", (clock() - decode_start_time) * 1000 / CLOCKS_PER_SEC);
		std::println("Image size: {}x{}", image->width, image->height);
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

		cinfo.image_width = image->width;
		cinfo.image_height = image->height;
		cinfo.input_components = 3;
		cinfo.in_color_space = JCS_RGB;

		jpeg_set_defaults(&cinfo);
		jpeg_set_quality(&cinfo, quality, TRUE);

		jpeg_start_compress(&cinfo, TRUE);

		JSAMPROW row_pointer[1];

		while (cinfo.next_scanline < cinfo.image_height)
		{
			if (cinfo.next_scanline % 100 == 0)
			{
				std::println("Encoding... {}/{}", cinfo.next_scanline, cinfo.image_height);
			}

			row_pointer[0] = (JSAMPROW) & (*image)[0, cinfo.next_scanline];
			jpeg_write_scanlines(&cinfo, row_pointer, 1);
		}

		jpeg_finish_compress(&cinfo);
		jpeg_destroy_compress(&cinfo);

		encoded_data.assign(out_buffer, out_buffer + out_size);

		free(out_buffer);
		std::println("Encoding Done in {} ms", (uint64_t)(clock() - encode_start_time) * 1000 / CLOCKS_PER_SEC);
		std::println("Output JPG size: {} bytes", encoded_data.size());
	}

	std::println("Writing to disk...");
	{
		auto* file = fopen("sd:/out13.jpg", "w");
		if (file == nullptr)
		{
			printf("Failed to open file\n");
			return 2;
		}

		fseek(file, 0, SEEK_SET);
		const size_t write_size = encoded_data.size();
		const auto actual_write = fwrite(encoded_data.data(), 1, write_size, file);

		if (actual_write != write_size)
		{
			printf(
				"Failed to write file, expected %zu bytes, written %zu bytes; errno=%d, FRESULT=%s\n",
				write_size,
				actual_write,
				errno,
				fresult_string[driver::fat32::last_failure]
			);
		}

		fclose(file);
		std::println("Writing done");
	}

	std::println("Read back and check...");
	{
		auto* file = fopen("sd:/out13.jpg", "r");
		if (file == nullptr)
		{
			printf("Failed to open file\n");
			return 1;
		}

		fseek(file, 0, SEEK_END);
		const size_t file_size = ftell(file);
		fseek(file, 0, SEEK_SET);

		std::vector<uint8_t> read_back_data(file_size);
		const auto read = fread(read_back_data.data(), 1, file_size, file);

		if (read != file_size)
		{
			printf("Failed to read file\n");
			return 1;
		}

		fclose(file);

		if (read_back_data.size() != encoded_data.size())
		{
			printf("Read back size mismatch\n");
			return 1;
		}

		for (size_t i = 0; i < read_back_data.size(); i++)
		{
			if (read_back_data[i] != encoded_data[i])
			{
				printf("Read back data mismatch at %zu\n", i);

				const auto min_idx = std::max(0, (int)i - 10);
				const auto max_idx = std::min<int>((int)read_back_data.size(), i + 10);

				printf("Expected: ");
				for (int j = min_idx; j < max_idx; j++)
				{
					printf("%02X ", encoded_data[j]);
				}
				printf("\n");

				printf("Actual:   ");
				for (int j = min_idx; j < max_idx; j++)
				{
					printf("%02X ", read_back_data[j]);
				}
				printf("\n");

				return 1;
			}
		}

		std::println("Read back and check done");
	}

	driver::fat32::unmount_disk();

	return 0;
}