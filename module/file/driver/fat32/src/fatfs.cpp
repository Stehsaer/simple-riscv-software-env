#include "module/file/driver/fatfs.hpp"

#include <sys/fcntl.h>

namespace file::driver::fatfs
{
	std::unique_ptr<Media_interface> media_interface = nullptr;
	std::unique_ptr<FATFS> fs = nullptr;
	FRESULT last_failure = FR_OK;

	static error_t convert_fat32_errcode(FRESULT result)
	{
		last_failure = result;

		switch (result)
		{
		case FR_OK:
			return 0;
			break;
		case FR_NO_FILE:
			return ENOENT;
		case FR_NO_PATH:
			return ENOTDIR;
		case FR_INVALID_NAME:
			return EINVAL;
		case FR_EXIST:
			return EEXIST;
		case FR_DENIED:
			return EACCES;
		case FR_TOO_MANY_OPEN_FILES:
			return EMFILE;
		case FR_LOCKED:
			return EBUSY;
		case FR_NOT_ENOUGH_CORE:
			return ENOMEM;
		case FR_DISK_ERR:
		case FR_MKFS_ABORTED:
		case FR_NOT_READY:
		case FR_INT_ERR:
			return EIO;
		case FR_TIMEOUT:
			return ETIMEDOUT;
		case FR_WRITE_PROTECTED:
			return EROFS;
		case FR_INVALID_DRIVE:
		case FR_NOT_ENABLED:
		case FR_NO_FILESYSTEM:
			return ENODEV;
		case FR_INVALID_OBJECT:
		case FR_INVALID_PARAMETER:
			return EINVAL;
		}

		return EINVAL;
	}

	std::pair<error_t, std::unique_ptr<file::File_descriptor>> Device::open(
		std::string_view path,
		int flags,
		mode_t mode [[maybe_unused]]
	) noexcept
	{
		std::unique_ptr<Fd> fd = std::make_unique<Fd>();

		BYTE fat32_mode;

		switch (flags & 0x3)
		{
		case O_RDONLY:
			fat32_mode = FA_READ;
			break;
		case O_WRONLY:
			fat32_mode = FA_WRITE;
			break;
		case O_RDWR:
			fat32_mode = FA_READ | FA_WRITE;
			break;
		default:
			return {EINVAL, nullptr};
		}

		if (flags & O_CREAT) fat32_mode |= FA_CREATE_ALWAYS;
		if (flags & O_EXCL) fat32_mode |= FA_CREATE_NEW;
		if (flags & O_TRUNC) fat32_mode |= FA_CREATE_ALWAYS;

		const auto fresult = f_open(&fd->file, path.data(), fat32_mode);
		const auto result = convert_fat32_errcode(fresult);

		if (result != 0) return {result, nullptr};

		return {0, std::move(fd)};
	}

	error_t Device::stat(std::string_view path, struct stat& info) noexcept
	{
		FILINFO f_info;

		const auto result = convert_fat32_errcode(f_stat(path.data(), &f_info));
		if (result != 0) return result;

		info.st_mode = f_info.fattrib & AM_DIR ? S_IFDIR : S_IFREG;
		info.st_size = f_info.fsize;
		info.st_blksize = 512;
		info.st_blocks = (info.st_size + 511) / 512;

		return 0;
	}

	error_t Device::unlink(std::string_view path) noexcept
	{
#if FF_FS_READONLY == 1
		return EROFS;
#endif

		return convert_fat32_errcode(f_unlink(path.data()));
	}

	error_t Device::link(std::string_view oldname, std::string_view newname) noexcept
	{
#if FF_FS_READONLY == 1
		return EROFS;
#endif

		return convert_fat32_errcode(f_rename(oldname.data(), newname.data()));
	}

	int Device::rename(std::string_view oldpath, std::string_view newpath) noexcept
	{
#if FF_FS_READONLY == 1
		errno = EROFS;
		return -1;
#endif

		const auto result = f_rename(oldpath.data(), newpath.data());
		if (result != FR_OK)
		{
			errno = convert_fat32_errcode(result);
			return -1;
		}

		return 0;
	}

	off_t Device::Fd::lseek(off_t offset, int whence) noexcept
	{
		FRESULT result;

		switch (whence)
		{
		case SEEK_SET:
			result = f_lseek(&file, offset);
			break;
		case SEEK_CUR:
			result = f_lseek(&file, f_tell(&file) + offset);
			break;
		case SEEK_END:
			result = f_lseek(&file, f_size(&file) + offset);
			break;
		default:
			return -1;
		}

		if (result != FR_OK)
		{
			errno = convert_fat32_errcode(result);
			return -1;
		}

		return f_tell(&file);
	}

	ssize_t Device::Fd::read(void* buf, size_t count) noexcept
	{
		UINT read_count;
		const auto result = f_read(&file, buf, count, &read_count);

		if (result != FR_OK)
		{
			errno = convert_fat32_errcode(result);
			return -1;
		}

		return read_count;
	}

	ssize_t Device::Fd::write(const void* buf, size_t count) noexcept
	{
#if FF_FS_READONLY == 1
		errno = EROFS;
		return -1;
#endif

		UINT write_count;
		const auto result = f_write(&file, buf, count, &write_count);

		if (result != FR_OK)
		{
			errno = convert_fat32_errcode(result);
			return -1;
		}

		return write_count;
	}

	error_t Device::Fd::fstat(struct stat& info [[maybe_unused]]) noexcept
	{
		return ENOTSUP;
	}

	int Device::Fd::ftruncate(off_t length [[maybe_unused]]) noexcept
	{
#if FF_FS_READONLY == 1
		errno = EROFS;
		return -1;
#endif

		const auto result = f_truncate(&file);

		if (result != FR_OK)
		{
			errno = convert_fat32_errcode(result);
			return -1;
		}

		return 0;
	}

	int Device::Fd::fsync() noexcept
	{
#if FF_FS_READONLY == 1
		errno = EROFS;
		return -1;
#endif

		const auto result = f_sync(&file);
		if (result != FR_OK)
		{
			errno = convert_fat32_errcode(result);
			return -1;
		}

		return 0;
	}

	bool Device::Fd::isatty() noexcept
	{
		errno = ENOTTY;
		return false;
	}

	error_t Device::Fd::close() noexcept
	{
		const auto result = f_close(&file);
		if (result != FR_OK) return convert_fat32_errcode(result);

		return 0;
	}

	std::optional<FRESULT> mount_disk()
	{
		if (media_interface == nullptr) return FR_INVALID_DRIVE;

		if (fs == nullptr)
		{
			fs = std::make_unique<FATFS>();
		}

		const auto result = f_mount(fs.get(), "", 1);

		if (result != FR_OK)
		{
			fs.reset();
			return result;
		}

		return std::nullopt;
	}

	void unmount_disk()
	{
		if (fs == nullptr) return;
		f_mount(nullptr, "", 0);
		fs.reset();
	}

}

extern "C"
{
	DSTATUS disk_initialize(BYTE pdrv [[maybe_unused]])
	{
		if (file::driver::fatfs::media_interface == nullptr) return STA_NOINIT;
		return file::driver::fatfs::media_interface->disk_initialize();
	}

	DSTATUS disk_status(BYTE pdrv [[maybe_unused]])
	{
		if (file::driver::fatfs::media_interface == nullptr) return STA_NOINIT;
		return file::driver::fatfs::media_interface->disk_status();
	}

	DRESULT disk_read(BYTE pdrv [[maybe_unused]], BYTE* buff, LBA_t sector, UINT count)
	{
		if (file::driver::fatfs::media_interface == nullptr) return RES_NOTRDY;
		return file::driver::fatfs::media_interface->disk_read(buff, sector, count);
	}

	DRESULT disk_write(BYTE pdrv [[maybe_unused]], const BYTE* buff, LBA_t sector, UINT count)
	{
		if (file::driver::fatfs::media_interface == nullptr) return RES_NOTRDY;
		return file::driver::fatfs::media_interface->disk_write(buff, sector, count);
	}

	DRESULT disk_ioctl(BYTE pdrv [[maybe_unused]], BYTE cmd, void* buff)
	{
		if (file::driver::fatfs::media_interface == nullptr) return RES_NOTRDY;
		return file::driver::fatfs::media_interface->disk_ioctl(cmd, buff);
	}
}