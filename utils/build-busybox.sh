# Builds busybox and the filesystem needed for linux on qemu
# Taken from: https://risc-v-machines.readthedocs.io/en/latest/linux/simple/

git clone https://git.busybox.net/busybox
pushd busybox
git checkout 1_32_1

make ARCH=riscv CROSS_COMPILE=riscv32-unknown-linux-gnu- defconfig
make ARCH=riscv CROSS_COMPILE=riscv32-unknown-linux-gnu- menuconfig
make ARCH=riscv CROSS_COMPILE=riscv64-unknown-linux-gnu- -j $(nproc)

mkdir initramfs
pushd initramfs
mkdir -p {bin,sbin,dev,etc,home,mnt,proc,sys,usr,tmp}
mkdir -p usr/{bin,sbin}
mkdir -p proc/sys/kernel
pushd dev
sudo mknod sda b 8 0 
sudo mknod console c 5 1
popd

cp ../busybox/busybox ./bin/

touch init

cat <<EOT >> init
#!/bin/busybox sh

# Make symlinks
/bin/busybox --install -s

# Mount system
mount -t devtmpfs  devtmpfs  /dev
mount -t proc      proc      /proc
mount -t sysfs     sysfs     /sys
mount -t tmpfs     tmpfs     /tmp

# Busybox TTY fix
setsid cttyhack sh

# https://git.busybox.net/busybox/tree/docs/mdev.txt?h=1_32_stable
echo /sbin/mdev > /proc/sys/kernel/hotplug
mdev -s

sh

EOT

chmod +x init

find . -print0 | cpio --null -ov --format=newc | gzip -9 > initramfs.cpio.gz

popd
