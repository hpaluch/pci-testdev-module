/*
 * pci-testdev.c - kernel module to test QEMU "PCI test device"
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>

#define MOD_NAME "pci-testdev"
#define MOD_VER  "0.0.1"

static int __init pci_testdev_init(void)
{
	pr_info("%s: v%s loaded\n",MOD_NAME,MOD_VER);
	return 0;
}
static void __exit pci_testdev_exit(void)
{
	pr_info("%s: v%s unloaded\n",MOD_NAME,MOD_VER);
}

module_init(pci_testdev_init);
module_exit(pci_testdev_exit);

MODULE_VERSION(MOD_VER);
MODULE_DESCRIPTION("Tester of QEMU PCI test device");
MODULE_AUTHOR("Henryk Paluch");
MODULE_LICENSE("GPL v2");
