#!/bin/bash
set -euo pipefail
set -x

cd $(dirname $0)
IMAGE=`pwd`/openSUSE-Leap-15.3.x86_64-NoCloud.qcow2

qemu-system-x86_64 -M pc -m 1024M \
  -drive file=$IMAGE,if=virtio,format=qcow2 \
  -net nic,model=virtio -net user -enable-kvm \
  -device pci-testdev \
  -display none -serial stdio -monitor none \
  -virtfs local,path=/home,mount_tag=host-home,security_model=passthrough
exit 0

#  -kernel /boot/vmlinuz-$(uname -r) \
#  -initrd /boot/initrd.img-$(uname -r) \
#  -append "ro root=/dev/vda1 console=ttyS0"
