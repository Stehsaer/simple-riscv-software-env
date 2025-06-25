dd if=/dev/zero of=$1 bs=1G count=1
sudo mkfs.fat -F 32 $1
