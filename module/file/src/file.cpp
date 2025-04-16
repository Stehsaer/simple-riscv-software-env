#include "file/interface/file.hpp"
#include "file/interface/guard.hpp"

#include <fcntl.h>
#include <sys/unistd.h>

namespace file
{
	namespace util
	{
		Path_result split_path(std::string_view path)
		{
			const auto sep_pos = path.find_first_of('/');

			Path_result result;

			if (sep_pos == std::string_view::npos)
			{
				result.root = path;
				result.rest = {};
			}
			else
			{
				result.root = path.substr(0, sep_pos);
				result.rest = path.substr(sep_pos + 1);
			}

			result.root_is_device = result.root.ends_with(":");

			if (result.root_is_device) result.root.remove_suffix(1);

			return result;
		}

		bool check_name_valid(std::string_view name)
		{
			return name.length() > 0
				&& std::find_if(
					   name.begin(),
					   name.end(),
					   [](char c)
					   {
						   return !std::isalnum(c) && c != '.' && c != '-' && c != '_';
					   }
				   ) == name.end();
		}
	}

#pragma region Filesystem_interface

	Dev_id_t Filesystem_interface::get_available_dev_id() const
	{
		if (devices.empty()) return 1;

		auto it_prev = devices.begin();
		auto it = devices.begin();
		++it;

		while (it != devices.end())
		{
			if (it->first - it_prev->first > 1) return it_prev->first + 1;

			++it_prev;
			++it;
		}

		return devices.rbegin()->first + 1;
	}

	Fd_t Filesystem_interface::get_available_fd() const
	{
		if (file_descriptor_map.empty()) return 3;

		const auto find_minimum = std::find(file_descriptor_map.begin(), file_descriptor_map.end(), nullptr);

		if (find_minimum == file_descriptor_map.end()) return file_descriptor_map.size();

		return find_minimum - file_descriptor_map.begin();
	}

	Filesystem_interface::File_descriptor_container* Filesystem_interface::get_file_descriptor(Fd_t fd)
	{
		if (fd < 0 || (size_t)fd >= file_descriptor_map.size()) return nullptr;
		auto* const find = file_descriptor_map[fd];
		return find;
	}

	File_device* Filesystem_interface::get_device(Dev_id_t dev_id)
	{
		const auto find = devices.find(dev_id);
		if (find == devices.end()) return nullptr;
		return find->second.device.get();
	}

	Dev_id_t Filesystem_interface::get_device_id(std::string_view name)
	{
		const auto find = device_name_map.find(name);
		if (find == device_name_map.end()) return -1;
		return find->second;
	}

	error_t Filesystem_interface::mount_device(Dev_id_t& dev_id, std::string_view path, std::unique_ptr<File_device> device)
	{
		if (!fs_available) return 0;

		// Step 1: Split path and check
		const auto path_result = util::split_path(path);

		if (!path_result.root_is_device) return ENOENT;
		if (!util::check_name_valid(path_result.root) || !path_result.rest.empty()) return EINVAL;

		// Step 2: Check if device already exists
		if (device_name_map.contains(path_result.root)) return EEXIST;

		// Step 3: Get device id
		dev_id = get_available_dev_id();

		// Step 4: Insert device
		devices.emplace(dev_id, std::move(device));
		device_name_map.emplace(path_result.root, dev_id);

		return 0;
	}

	error_t Filesystem_interface::mount_device(std::string_view path, std::unique_ptr<File_device> device)
	{
		if (!fs_available) return 0;

		// Step 1: Split path and check
		const auto path_result = util::split_path(path);

		if (!path_result.root_is_device) return ENOENT;
		if (!util::check_name_valid(path_result.root) || !path_result.rest.empty()) return EINVAL;

		// Step 2: Check if device already exists
		if (device_name_map.contains(path_result.root)) return EEXIST;

		// Step 3: Get device id
		const auto dev_id = get_available_dev_id();

		// Step 4: Insert device
		devices.emplace(dev_id, std::move(device));
		device_name_map.emplace(path_result.root, dev_id);

		return 0;
	}

	error_t Filesystem_interface::unmount_device(std::string_view path)
	{
		if (!fs_available) return 0;

		// Step 1: Split path and check
		const auto path_result = util::split_path(path);

		if (!path_result.root_is_device) return ENOENT;
		if (!util::check_name_valid(path_result.root) || !path_result.rest.empty()) return EINVAL;

		// Step 2: Get device
		const auto dev_id = device_name_map.find(path_result.root);
		if (dev_id == device_name_map.end()) return ENOENT;

		// Step 3: Check if device exists
		const auto device = devices.find(dev_id->second);

		// Step 4: Check if device is busy
		if (device->second.fd_count > 0) return EBUSY;

		// Step 5: Remove device
		device_name_map.erase(dev_id);
		devices.erase(device);

		return 0;
	}

	off_t Filesystem_interface::lseek(Fd_t fd, off_t offset, int whence)
	{
		if (!fs_available)
		{
			errno = EBADF;
			return -1;
		}

		auto* fd_ptr = get_file_descriptor(fd);
		if (fd_ptr == nullptr)
		{
			errno = EBADF;
			return -1;
		}

		return fd_ptr->descriptor->lseek(offset, whence);
	}

	ssize_t Filesystem_interface::read(Fd_t fd, void* buf, size_t count)
	{

		if (!fs_available)
		{
			errno = EBADF;
			return -1;
		}

		auto* fd_ptr = get_file_descriptor(fd);
		if (fd_ptr == nullptr)
		{
			errno = EBADF;
			return -1;
		}

		if (!fd_ptr->read)
		{
			errno = EBADF;
			return -1;
		}

		return fd_ptr->descriptor->read(buf, count);
	}

	ssize_t Filesystem_interface::write(Fd_t fd, const void* buf, size_t count)
	{
		if (!fs_available)
		{
			errno = EBADF;
			return -1;
		}

		auto* fd_ptr = get_file_descriptor(fd);
		if (fd_ptr == nullptr)
		{
			errno = EBADF;
			return -1;
		}

		if (!fd_ptr->write)
		{
			errno = EBADF;
			return -1;
		}

		return fd_ptr->descriptor->write(buf, count);
	}

	error_t Filesystem_interface::fstat(Fd_t fd, struct stat& info)
	{
		if (!fs_available) return EBADF;

		auto* fd_ptr = get_file_descriptor(fd);
		if (fd_ptr == nullptr) return EBADF;

		const auto errno1 = fd_ptr->descriptor->fstat(info);

		if (errno1 != 0) return errno1;

		info.st_dev = fd_ptr->dev_id;
		info.st_rdev = fd_ptr->dev_id;

		return 0;
	}

	bool Filesystem_interface::isatty(Fd_t fd)
	{
		if (!fs_available)
		{
			errno = EBADF;
			return false;
		}

		auto* fd_ptr = get_file_descriptor(fd);

		if (fd_ptr == nullptr)
		{
			errno = EBADF;
			return false;
		}

		return fd_ptr->descriptor->isatty();
	}

	error_t Filesystem_interface::close(Fd_t fd)
	{
		if (!fs_available) return 0;

		// Step 1: Get file decriptior pointer
		auto* fd_ptr = get_file_descriptor(fd);
		if (fd_ptr == nullptr) return EBADF;

		// Step 2: Close file descriptor
		if (fd_ptr->ref_count == 1)
		{
			// Last reference, close underlying file descriptor

			auto* const descriptor = fd_ptr->descriptor.get();
			if (descriptor == nullptr) return EFAULT;

			const auto close_result = descriptor->close();
			if (close_result != 0) return close_result;

			const auto find = std::find_if(
				file_descriptors.begin(),
				file_descriptors.end(),
				[fd_ptr](const auto& ptr) -> bool
				{
					return ptr.get() == fd_ptr;
				}
			);

			if (find != file_descriptors.end())
				file_descriptors.erase(find);
			else
				return EFAULT;
		}
		else
		{
			fd_ptr->ref_count--;
		}

		file_descriptor_map[fd] = is_reserved_fd(fd) ? empty_fd.get() : nullptr;

		if (is_reserved_fd(fd)) empty_fd->ref_count++;

		return 0;
	}

	int Filesystem_interface::ftruncate(Fd_t fd, off_t length)
	{
		if (!fs_available)
		{
			errno = EBADF;
			return -1;
		}

		auto* fd_ptr = get_file_descriptor(fd);
		if (fd_ptr == nullptr)
		{
			errno = EBADF;
			return -1;
		}

		return fd_ptr->descriptor->ftruncate(length);
	}

	Fd_t Filesystem_interface::dup(Fd_t oldfd)
	{
		const auto avail_fd = get_available_fd();
		if ((size_t)avail_fd >= file_descriptor_map.size()) file_descriptor_map.resize(avail_fd + 1, nullptr);

		auto* fd_ptr = get_file_descriptor(oldfd);
		if (fd_ptr == nullptr) return EBADF;

		if (fd_ptr->write) return EBUSY;

		fd_ptr->ref_count++;
		file_descriptor_map[avail_fd] = fd_ptr;

		return 0;
	}

	Fd_t Filesystem_interface::dup2(Fd_t oldfd, Fd_t newfd)
	{
		if (oldfd == newfd) return newfd;

		if (newfd < 0) return EBADF;
		if ((size_t)newfd >= file_descriptor_map.size()) file_descriptor_map.resize(newfd + 1, nullptr);

		auto* fd_ptr = get_file_descriptor(oldfd);
		if (fd_ptr == nullptr) return EBADF;

		if (fd_ptr->write) return EBUSY;

		if (file_descriptor_map[newfd] != nullptr)
		{
			const auto close_result = this->close(newfd);
			if (close_result != 0) return close_result;
		}

		fd_ptr->ref_count++;
		file_descriptor_map[newfd] = fd_ptr;

		return newfd;
	}

	int Filesystem_interface::open(Fd_t& fd, std::string_view path, int flags, mode_t mode)
	{
		if (!fs_available) return 0;

		// Step 1: Split path and check if start with root
		const auto path_result = util::split_path(path);
		if (!path_result.root_is_device)
		{
			errno = EINVAL;
			return -1;
		}

		// Step 2: Name -> Id -> Device
		const auto device_id = get_device_id(path_result.root);
		if (device_id == -1)
		{
			errno = ENOENT;
			return -1;
		}

		auto* dev = get_device(device_id);
		if (dev == nullptr)
		{
			errno = EFAULT;
			return -1;
		}  // Shouldn't be nullptr. If yes, internal error is suggested.

		// Step 3: Pass open parameter to the device (Must be done before actually allocating stuffs)
		auto [err, file_descriptor] = dev->open(path_result.rest, flags, mode);
		if (err != 0)
		{
			errno = err;
			return -1;
		}

		// Step 4: Acquire available fd
		const auto avail_fd = get_available_fd();
		if ((size_t)avail_fd >= file_descriptor_map.size()) file_descriptor_map.resize(avail_fd + 1, nullptr);

		// Step 5: Add fd to list
		fd = avail_fd;
		const auto [iter, success] = file_descriptors.emplace(std::make_unique<File_descriptor_container>(
			std::move(file_descriptor),
			1,
			device_id,
			(flags & 0x3) == O_RDONLY || (flags & 0x3) == O_RDWR,
			(flags & 0x3) == O_WRONLY || (flags & 0x3) == O_RDWR
		));

		if (!success)
		{
			errno = EFAULT;
			return -1;
		}  // Shouldn't fail. If failed, internal error is suggested.

		file_descriptor_map[fd] = iter->get();

		return 0;
	}

	error_t Filesystem_interface::stat(std::string_view path, struct stat& info)
	{
		if (!fs_available) return 0;

		// Step 1: Split path and check if start with root
		const auto path_result = util::split_path(path);
		if (!path_result.root_is_device) return EINVAL;

		// Step 2: Name -> Id -> Device
		const auto device_id = get_device_id(path_result.root);
		if (device_id == -1) return ENOENT;
		auto* dev = get_device(device_id);
		if (dev == nullptr) return EFAULT;  // Shouldn't be nullptr. If yes, internal error is suggested.

		const auto result = dev->stat(path_result.rest, info);
		if (result != 0) return result;

		info.st_dev = device_id;
		info.st_rdev = device_id;

		return 0;
	}

	error_t Filesystem_interface::unlink(std::string_view path)
	{
		if (!fs_available) return 0;

		// Step 1: Split path and check if start with root
		const auto path_result = util::split_path(path);
		if (!path_result.root_is_device) return EINVAL;

		// Step 2: Name -> Id -> Device
		const auto device_id = get_device_id(path_result.root);
		if (device_id == -1) return ENOENT;
		auto* dev = get_device(device_id);
		if (dev == nullptr) return EFAULT;  // Shouldn't be nullptr. If yes, internal error is suggested.

		return dev->unlink(path_result.rest);
	}

	error_t Filesystem_interface::link(std::string_view oldname, std::string_view newname)
	{
		if (!fs_available) return 0;

		// Step 1: Split path and check if start with root
		const auto oldname_result = util::split_path(oldname);
		if (!oldname_result.root_is_device) return EINVAL;

		const auto newname_result = util::split_path(newname);
		if (!newname_result.root_is_device) return EINVAL;

		// Step 2: Name -> Id -> Device
		const auto old_device_id = get_device_id(oldname_result.root);
		const auto new_device_id = get_device_id(newname_result.root);
		if (old_device_id == -1) return ENOENT;
		if (new_device_id == -1) return ENOENT;
		if (old_device_id != new_device_id) return EXDEV;

		auto* dev = get_device(old_device_id);
		if (dev == nullptr) return EFAULT;  // Shouldn't be nullptr. If yes, internal error is suggested.

		return dev->link(oldname_result.rest, newname_result.rest);
	}

#pragma endregion
}