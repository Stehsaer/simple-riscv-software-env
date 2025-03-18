#include "os/file.hpp"

namespace qemu_device
{
	struct Serial
	{
		uint8_t trx;  // TX or RX for NS16550

		class Device : public os::filesystem::File_device
		{
			volatile Serial& serial;

		  public:

			Device(volatile Serial& uart) :
				serial(uart)
			{
			}

			std::pair<error_t, std::unique_ptr<os::filesystem::File_descriptor>> open(
				std::string_view path,
				int flags,
				mode_t mode
			) noexcept override;

			error_t stat(std::string_view path, struct stat& info) noexcept override;
			error_t unlink(std::string_view path) noexcept override;
			error_t link(std::string_view oldname, std::string_view newname) noexcept override;
			int rename(std::string_view oldpath, std::string_view newpath) noexcept override;
		};

		class Fd : public os::filesystem::File_descriptor
		{
			volatile Serial& serial;

		  public:

			Fd(volatile Serial& uart) :
				serial(uart)
			{
			}

			off_t lseek(off_t offset, int whence) noexcept override;
			ssize_t read(void* buf, size_t count) noexcept override;
			ssize_t write(const void* buf, size_t count) noexcept override;
			error_t fstat(struct stat& info) noexcept override;
			bool isatty() noexcept override;
			error_t close() noexcept override;
			error_t fsync() noexcept override;
			int ftruncate(off_t length) noexcept override;
		};

		std::unique_ptr<os::filesystem::File_device> get_device() volatile { return std::make_unique<Device>(*this); }
	};

	extern volatile Serial& serial;
}