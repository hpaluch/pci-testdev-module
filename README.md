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

Early work in progress - no useful code yet...

# Building

I build it on host openSUSE 15.3 LEAP. Install these packages for building:
```bash
sudo zypper in kernel-default-devel gcc make
```

And then just issue:
```bash
make
```

# Testing

You need same Linux version and kernel running under QEMU (because this device
is available only for QEMU guests).
Example of running such QEMU guest is in script `vm/run_qemu_suse.sh`

If guest works properly you can do this in guest:
```bash
# openSUSE QEMU Guest

insmod /mnt/host-home/USER/projects/pci-testdev-module/pci-testdev.ko
dmesg

...
pci_testdev: loading out-of-tree module taints kernel.
pci_testdev: module verification failed: signature and/or required key missing - tainting kernel
pci-testdev: v0.0.1 loaded
...
```

And unloading module:
```bash
# openSUSE QEMU Guest

rmmod pci-testdev
dmesg

...
pci-testdev: v0.0.1 unloaded
...
```

