# Test QEMU pci-testdev

Here is Linux kernel module to test QEMU PCI Test Dev (it is completely
virtual PCI device available only for QEMU guests used just
for testing).

You can see example how to run QEMU with PCI Test Dev example
in `vm/run_qemu_suse.sh` - the parameter `-device pci-testdev`

More on PCI Test device is in QEMU source tree:
- https://github.com/qemu/qemu/blob/master/docs/specs/pci-testdev.txt
- https://github.com/qemu/qemu/blob/master/docs/specs/pci-ids.txt
Later has this important line:
```
1b36:0005  PCI test device (docs/specs/pci-testdev.txt)
```

# Status

Early work in progress:
* register/unregister device class `/sys/class/pci-testdev`
* registered PCI probe/remove (not finished)
* alloc PCI structure per card (device)
* enable PCI card

# Preparing QEMU VM

At first download QEMU QCOW2 disk image. You can use script:
```bash
vm/download_suse_cloud.sh
```
Now we need to set root's password in this VM (it is locked). We will use
this trick to mount such image:
```bash
sudo modprobe nbd max_part=8
sudo qemu-nbd --connect=/dev/nbd0 `pwd`/vm/openSUSE-Leap-15.3.x86_64-NoCloud.qcow2
sudo mkdir /mnt/sysimage
sudo mount -t xfs /dev/nbd0p3 /mnt/sysimage
```
Now enter chroot into this image:
```bash
chroot /mnt/sysimage
```
Setup root's password:
```bash
# inside chroot!
passwd
```
Also add line from `vm/fstab.addon` to `/etc/fstab` of this VM.
And create directory for mount
```bash
# inside chroot!
mkdir /mnt/host-home
```
Now we can exit chroot and unmount image:
```bash
# inside chroot!
Ctrl-d # to exit chroot
sudo umount /mnt/sysimage
sudo qemu-nbd -d /dev/nbd0
```
Our SUSE VM should be ready.

# Building

I build it on host openSUSE 15.3 LEAP. Install these packages for building:
```bash
sudo zypper in kernel-default-devel gcc make
```

And then just issue:
```bash
make
```
It should create module file `pci-testdev.ko`.

# Testing

You need same Linux version and kernel running under QEMU (because this device
is available only for QEMU guests).

To verify that you have same kernels run `name -r` both in SUSE host
and guest. They must be same.

Example of running such QEMU guest is in script `vm/run_qemu_suse.sh`

If guest works properly you can do this in guest:
```bash
# openSUSE QEMU Guest
# enable debug level in printk output:
echo 8 > /proc/sys/kernel/printk

insmod /mnt/host-home/USER/projects/pci-testdev-module/pci-testdev.ko
dmesg

...
pci_testdev: loading out-of-tree module taints kernel.
pci_testdev: module verification failed: signature and/or required key missing - tainting kernel

pci-testdev: pci_testdev_init:139 - ENTER
pci-testdev: registered class '/sys/class/pci-testdev'
pci-testdev 0000:00:04.0: pci_testdev_probe:75 PCI ID 1b36:0005 SSID 1af4:1100 - ENTER
pci-testdev 0000:00:04.0: OK PCI card enabled
pci-testdev 0000:00:04.0: OK PCI ID 1b36:0005 SSID 1af4:1100 adding instance 0x000000001e53f4e1
pci-testdev 0000:00:04.0: pci_testdev_probe:108 - EXIT, err=0
pci-testdev: registered PCI driver
pci-testdev: WARNING! Individual card probe may still fail!
pci-testdev: v0.0.5 loaded DEBUG mode
pci-testdev: pci_testdev_init:169 - EXIT, err=0
...
```

And unloading module:
```bash
# openSUSE QEMU Guest

rmmod pci-testdev
dmesg

...
pci-testdev 0000:00:04.0: pci_testdev_remove:54 OK - PCI card disabled
pci-testdev 0000:00:04.0: pci_testdev_remove:58 Freeing 000000001e53f4e1
pci-testdev 0000:00:04.0: pci_testdev_remove:64 - EXIT
pci-testdev: unregistered PCI driver
pci-testdev: released class '/sys/class/pci-testdev'
pci-testdev: v0.0.5 unloaded
pci-testdev: pci_testdev_exit:177 - EXIT
...
```
# Using Eclipse CDT IDE

You can basically follow:
- https://wiki.eclipse.org/HowTo_use_the_CDT_to_navigate_Linux_kernel_source
With these changes
- use Import ... -> Existing Makefile Project -> and C and `GCC Linux` compiler
- I recommend to create `KDIR` variable under Project -> Properties ->
  C++ Build -> Build Variables pointing to `/usr/src/linux`
- and then use above `KDIR` for all include paths in kernel source
- you need to additionally define symbol `MODULE`

# Resources
* Device Driver Design Patterns - how to support more than 1 PCI card...
  - https://docs.kernel.org/driver-api/driver-model/design-patterns.html
* How to build external modules:
  - https://docs.kernel.org/kbuild/modules.html
* How to prepare Eclipse CDT for kernel (module) development:
  - https://wiki.eclipse.org/HowTo_use_the_CDT_to_navigate_Linux_kernel_source
