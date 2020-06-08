#!/bin/sh
set -e
. ./iso.sh
objcopy --only-keep-debug ./sysroot/boot/metros.kernel kernel.sym
. ./qemu.sh

