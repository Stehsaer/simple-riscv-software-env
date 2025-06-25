PROJECT_DIR=$(pwd)/project/os-class/lab8
DISK_CONTENT_DIR=$(pwd)/disk-content

mkdir -p $DISK_CONTENT_DIR

if [ ! -f $(pwd)/disk.img ]; then
	./script/create-img.sh $(pwd)/disk.img
fi

riscv64-unknown-elf-g++ -march=rv32ima_zicond_zicsr_zifencei -mabi=ilp32 -o $PROJECT_DIR/a.out $PROJECT_DIR/main.cpp
cp $PROJECT_DIR/a.out $DISK_CONTENT_DIR

./script/disk-configure.sh $(pwd)/disk.img $DISK_CONTENT_DIR