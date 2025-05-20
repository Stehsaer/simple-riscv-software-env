#!/bin/bash

if [ "$1" -eq "-h"]; then
	echo "Usage: $0 <fat32_image_file> <dest_directory>"
	exit 0
fi

if [ "$#" -ne 2 ]; then
	echo "Usage: $0 <fat32_image_file> <dest_directory>"
	exit 1
fi

IMAGE_FILE="$1"
DEST_DIR="$2"
MOUNT_POINT="./temp"

if [ ! -d "$DEST_DIR" ]; then
	echo "Error: Source directory '$DEST_DIR' does not exist."
	exit 1
fi

if [ ! -d "$MOUNT_POINT" ]; then
	mkdir "$MOUNT_POINT"
fi

sudo mount -o loop,umask=000 "$IMAGE_FILE" "$MOUNT_POINT"
if [ $? -ne 0 ]; then
	echo "Error: Failed to mount '$IMAGE_FILE'."
	exit 1
fi

cp -r "$MOUNT_POINT"/* "$DEST_DIR"
if [ $? -ne 0 ]; then
	echo "Error: Failed to copy files to the mounted image."
	sudo umount "$MOUNT_POINT"
	exit 1
fi

sudo umount "$MOUNT_POINT"
if [ $? -ne 0 ]; then
	echo "Error: Failed to unmount '$MOUNT_POINT'."
	exit 1
fi

rmdir temp

echo "Files successfully copied to the FAT32 image."
