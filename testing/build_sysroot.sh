#!/bin/bash

mkdir -p $1/boot/grub/
mkdir -p $1/dev
sudo rm -f $1/dev/tty
sudo rm -f $1/dev/tty0
sudo rm -f $1/dev/null
sudo rm -f $1/dev/zero
sudo mknod $1/dev/tty c 4 0
sudo mknod $1/dev/tty0 c 5 0
sudo mknod $1/dev/null c 1 3
sudo mknod $1/dev/zero c 1 5
mkdir -p $1/usr/lib
mkdir -p $1/usr/include
