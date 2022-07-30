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



