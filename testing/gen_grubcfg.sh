#!/bin/bash

cat >$1 <<EOF
set default=0

menuentry 'My Kernel' {
	multiboot /boot/$2
	boot
}
EOF
