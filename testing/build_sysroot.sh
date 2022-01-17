#!/bin/bash

mkdir -p $1/boot/grub/
mkdir -p $1/dev
sudo rm -f $1/dev/tty
sudo rm -f $1/dev/tty0
sudo mknod $1/dev/tty c 4 0
sudo mknod $1/dev/tty0 c 5 0
