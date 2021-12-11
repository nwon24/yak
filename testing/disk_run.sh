#!/bin/sh

LOOP_UNUSED1=$(sudo losetup -f)
sudo losetup $LOOP_UNUSED1 $1 -o $[512*2048]
sudo mount $LOOP_UNUSED1 /mnt
sudo cp -rf $2/* /mnt
sudo sync
sudo umount /mnt
sudo losetup -d $LOOP_UNUSED1
qemu-system-i386 -hda $1 -d int,cpu_reset -D qemu.log
