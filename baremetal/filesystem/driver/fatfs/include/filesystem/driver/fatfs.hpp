#pragma once

#include "fatfs/diskio.h"
#include "fatfs/ff.h"

#include "filesystem/interface.hpp"

#include <memory>
#include <optional>

namespace filesystem::driver::fatfs
{
	class Media_interface
	{
	  public:

		virtual ~Media_interface() = default;

		virtual DSTATUS disk_initialize() = 0;
		virtual DSTATUS disk_status() = 0;
		virtual DRESULT disk_read(BYTE* buff, LBA_t sector, UINT count) = 0;
		virtual DRESULT disk_write(const BYTE* buff, LBA_t sector, UINT count) = 0;
		virtual DRESULT disk_ioctl(BYTE cmd, void* buff) = 0;
	};

	extern std::unique_ptr<Media_interface> media_interface;
	extern FRESULT last_failure;
	extern const char* last_failure_func;

	class Device : public filesystem::Device
	{
	  public:

		class Fd : public filesystem::File_descriptor
		{
		  public:

			FIL file;

			off_t lseek(off_t offset, int whence) noexcept override;
			ssize_t read(void* buf, size_t count) noexcept override;
			ssize_t write(const void* buf, size_t count) noexcept override;
			error_t fstat(struct stat& info) noexcept override;
			int fsync() noexcept override;
			int ftruncate(off_t length) noexcept override;
			bool isatty() noexcept override;
			error_t close() noexcept override;
		};

		Device() = default;

		std::pair<error_t, std::unique_ptr<filesystem::File_descriptor>> open(
			std::string_view path,
			int flags,
			mode_t mode
		) noexcept override;
		error_t stat(std::string_view path, struct stat& info) noexcept override;
		error_t unlink(std::string_view path) noexcept override;
		error_t link(std::string_view oldname, std::string_view newname) noexcept override;
		int rename(std::string_view oldpath, std::string_view newpath) noexcept override;
	};

	std::optional<FRESULT> mount_disk();
	void unmount_disk();

	inline std::pair<FRESULT, DIR> opendir(const TCHAR* path)
	{
		DIR dir;
		const auto result = f_opendir(&dir, path);

		return {result, dir};
	}

	inline std::pair<FRESULT, FILINFO> readdir(DIR& dp)
	{
		FILINFO fno;

		const auto result = f_readdir(&dp, &fno);

		return {result, fno};
	}

	inline FRESULT closedir(DIR& dp)
	{
		return f_closedir(&dp);
	}
}