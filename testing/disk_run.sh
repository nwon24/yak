#!/bin/bash

LOOP_UNUSED1=$(sudo losetup -f)
sudo losetup $LOOP_UNUSED1 $1 -o $[512*2048]
sudo fsck.ext2 -f -y -v $LOOP_UNUSED1
sudo mount $LOOP_UNUSED1 /mnt
sudo cp -rf $2/* /mnt
sudo sync
sudo umount /mnt
qemu-system-i386 -s -hda $1 -d int,cpu_reset -D qemu.log -no-reboot -no-shutdown -machine pc -monitor stdio
#sudo mount $LOOP_UNUSED1 /mnt
#sudo cp -rf /mnt/* $2/
#sudo umount /mnt
sudo losetup -d $LOOP_UNUSED1
