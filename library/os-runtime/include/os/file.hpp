#pragma once

#include <climits>
#include <cstdint>
#include <memory>
#include <set>
#include <string_view>
#include <sys/stat.h>
#include <vector>

#include "cpp-guard.h"
#include "util/map-stringview.hpp"

namespace os::filesystem
{
	namespace util
	{
		struct Path_result
		{
			std::string_view root, rest;
			bool root_is_device;
		};

		Path_result split_path(std::string_view path);

		bool check_name_valid(std::string_view name);
	}

	struct File_descriptor
	{
		virtual ~File_descriptor() = default;

		virtual off_t lseek(off_t offset, int whence) = 0;
		virtual ssize_t read(void* buf, size_t count) = 0;
		virtual ssize_t write(const void* buf, size_t count) = 0;
		virtual error_t fstat(struct stat& info) = 0;
		virtual int fsync() = 0;
		virtual int ftruncate(off_t length) = 0;
		virtual bool isatty() = 0;
		virtual error_t close() = 0;
	};

	struct File_device
	{
		virtual ~File_device() = default;

		virtual std::pair<error_t, std::unique_ptr<File_descriptor>> open(std::string_view path, int flags, mode_t mode) = 0;
		virtual error_t stat(std::string_view path, struct stat& info) = 0;
		virtual error_t unlink(std::string_view path) = 0;
		virtual error_t link(std::string_view oldname, std::string_view newname) = 0;
		virtual int rename(std::string_view oldpath, std::string_view newpath) = 0;
	};

	struct Empty_fd : public File_descriptor
	{
		off_t lseek(off_t, int) override { return ESPIPE; }
		ssize_t read(void*, size_t) override { return EBUSY; }
		ssize_t write(const void*, size_t) override { return EBUSY; }
		error_t fstat(struct stat&) override { return EBUSY; }
		int fsync() override
		{
			errno = EBUSY;
			return -1;
		}
		error_t close() override { return 0; }

		int ftruncate(off_t) override
		{
			errno = EINVAL;
			return -1;
		}

		bool isatty() override
		{
			errno = ENOTTY;
			return false;
		}
	};

	using Fd_t = int;
	using Dev_id_t = int;

	class Filesystem_interface
	{
		struct Device_container
		{
			std::unique_ptr<File_device> device;

			// Opened file descriptors count
			size_t fd_count = 0;
		};

		struct File_descriptor_container
		{
			std::unique_ptr<File_descriptor> descriptor;

			// Reference count, when decreased to 0, the base file descriptor is closed.
			size_t ref_count;

			// Binded device id
			Dev_id_t dev_id;

			// Modes
			bool read, write;
		};

		std::map<Dev_id_t, Device_container> devices;
		String_view_map<Dev_id_t> device_name_map;

		std::set<std::unique_ptr<File_descriptor_container>> file_descriptors;
		std::vector<File_descriptor_container*> file_descriptor_map;

		/* Helper Functions */

		// Get file descriptor from file descriptor table.
		// -- Return `nullptr` if not found.
		File_descriptor_container* get_file_descriptor(Fd_t fd);

		// Get device from id.
		// -- Return `nullptr` if not found.
		File_device* get_device(Dev_id_t dev_id);

		// Get device id from name.
		// -- Return `-1` if not found.
		Dev_id_t get_device_id(std::string_view name);

		Dev_id_t get_available_dev_id() const;
		Fd_t get_available_fd() const;

		static inline bool is_reserved_fd(Fd_t fd) { return fd < 3; }

		std::unique_ptr<File_descriptor_container> empty_fd;

	  public:

		Filesystem_interface()
		{
			empty_fd = std::make_unique<File_descriptor_container>(std::make_unique<Empty_fd>(), INT_MAX, 0, false, false);
			file_descriptor_map.resize(3, empty_fd.get());
		}

		/* Path-based API (POSIX) */

		int open(Fd_t& fd, std::string_view path, int flags, mode_t mode);
		error_t stat(std::string_view path, struct stat& info);
		error_t unlink(std::string_view path);
		error_t link(std::string_view oldname, std::string_view newname);
		int rename(std::string_view oldpath, std::string_view newpath);

		/* File Descriptor Based API (POSIX) */

		off_t lseek(Fd_t fd, off_t offset, int whence);
		ssize_t read(Fd_t fd, void* buf, size_t count);
		ssize_t write(Fd_t fd, const void* buf, size_t count);
		error_t fstat(Fd_t fd, struct stat& info);
		bool isatty(Fd_t fd);
		error_t close(Fd_t fd);
		int ftruncate(Fd_t fd, off_t length);

		Fd_t dup(Fd_t oldfd);
		Fd_t dup2(Fd_t oldfd, Fd_t newfd);

		/* Not in POSIX Standard */

		error_t mount_device(Dev_id_t& dev_id, std::string_view path, std::unique_ptr<File_device> device);
		error_t mount_device(std::string_view path, std::unique_ptr<File_device> device);
		error_t unmount_device(std::string_view path);
	};
}
