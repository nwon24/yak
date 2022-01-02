#!/bin/bash

dd if=/dev/zero of=$1 bs=1024 count=$2
printf ",,L,*" | sfdisk $1
LOOP_UNUSED1=$(sudo losetup -f)
sudo losetup $LOOP_UNUSED1 $1
LOOP_UNUSED2=$(sudo losetup -f)
sudo losetup $LOOP_UNUSED2 $1 -o $[512*2048]
sudo mkfs.ext2 $LOOP_UNUSED2
sudo mount $LOOP_UNUSED2 /mnt
sudo grub-install --target=i386-pc --no-floppy --root-directory=/mnt --modules="multiboot ext2 part_msdos" $LOOP_UNUSED1
sudo umount /mnt
sudo losetup -d $LOOP_UNUSED1
sudo losetup -d $LOOP_UNUSED2
