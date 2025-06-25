#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 2 ]; then
	echo "Usage: $0 <fat32_image_file> <source_directory>"
	exit 1
fi

IMAGE_FILE="$1"
SOURCE_DIR="$2"
MOUNT_POINT="./temp"

# Ensure the source directory exists
if [ ! -d "$SOURCE_DIR" ]; then
	echo "Error: Source directory '$SOURCE_DIR' does not exist."
	exit 1
fi

# Create the mount point if it doesn't exist
if [ ! -d "$MOUNT_POINT" ]; then
	mkdir "$MOUNT_POINT"
fi

# Mount the FAT32 image file
sudo mount -o loop,umask=000 "$IMAGE_FILE" "$MOUNT_POINT"
if [ $? -ne 0 ]; then
	echo "Error: Failed to mount '$IMAGE_FILE'."
	exit 1
fi

# Copy all files and directories from the source directory to the mount point
cp -r "$SOURCE_DIR"/* "$MOUNT_POINT"
if [ $? -ne 0 ]; then
	echo "Error: Failed to copy files to the mounted image."
	sudo umount "$MOUNT_POINT"
	exit 1
fi

# Unmount the image file
sudo umount "$MOUNT_POINT"
if [ $? -ne 0 ]; then
	echo "Error: Failed to unmount '$MOUNT_POINT'."
	exit 1
fi

rmdir temp

echo "Files successfully copied to the FAT32 image."
