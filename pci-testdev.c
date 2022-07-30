/*
 * pci-testdev.c - kernel module to test QEMU "PCI test device"
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/device.h>

#define MOD_SYS_CLASS_PATH "/sys/class"
#define MOD_CLASS "pci-testdev"
#define MOD_NAME "pci-testdev"
#define MOD_VER  "0.0.2"

static struct class *pci_testdev_class = NULL;

static void pci_testdev_cleanup(void)
{
	if (pci_testdev_class != NULL){
		class_destroy(pci_testdev_class);
		pci_testdev_class = NULL;
		pr_debug("%s: released class '%s/%s'\n",MOD_NAME,MOD_SYS_CLASS_PATH,MOD_CLASS);
	}
}

static int __init pci_testdev_init(void)
{
	int err = 0;

    pci_testdev_class = class_create(THIS_MODULE, MOD_CLASS);
    if (IS_ERR(pci_testdev_class)) {
             err = PTR_ERR(pci_testdev_class);
             pr_err("%s: class_create('%s'): failed with err=%d\n",MOD_NAME,MOD_CLASS,err);
             pci_testdev_class = NULL; // signal for cleanup pci_testdev_cleanup!
             goto exit0;
    }
	pr_debug("%s: registered class '%s/%s'\n",MOD_NAME,MOD_SYS_CLASS_PATH,MOD_CLASS);

	pr_info("%s: v%s loaded"
#ifdef DEBUG
			" DEBUG mode"
#endif
			"\n",MOD_NAME,MOD_VER);
exit0:
	if(err){
		pr_err("%s: init failed with err=%d\n",MOD_NAME,err);
		pci_testdev_cleanup();
	}
	return err;
}
static void __exit pci_testdev_exit(void)
{
	pci_testdev_cleanup();
	pr_info("%s: v%s unloaded\n",MOD_NAME,MOD_VER);
}

module_init(pci_testdev_init);
module_exit(pci_testdev_exit);

MODULE_VERSION(MOD_VER);
MODULE_DESCRIPTION("Tester of QEMU PCI test device");
MODULE_AUTHOR("Henryk Paluch");
MODULE_LICENSE("GPL v2");
