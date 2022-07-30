/*
 * pci-testdev.c - kernel module to test QEMU "PCI test device"
 * (C) Copyright Henryk Paluch
 *
 * Based on various parts of kernel code, including:
 * - drivers/misc/hpilo.c
 * - drivers/misc/pti.c
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/device.h>
#include <linux/pci.h>

#define MOD_PCI_VID 0x1b36
#define MOD_PCI_DID 0x0005
#define MOD_SYS_CLASS_PATH "/sys/class"
#define MOD_CLASS "pci-testdev"
#define MOD_NAME "pci-testdev"
#define MOD_VER  "0.0.3"

static struct class *pci_testdev_class = NULL;

static const struct pci_device_id pci_ids[] = {
                {PCI_DEVICE(MOD_PCI_VID, MOD_PCI_DID)},
                {0}
};

static int pci_testdev_probe(struct pci_dev *pdev,
		const struct pci_device_id *ent)
{
	int err = -EINVAL;

	dev_dbg(&pdev->dev, "%s(%d): my PCI ID %04x:%04x\n",
			__func__, __LINE__, pdev->vendor, pdev->device);
	pr_err("%s: %s not yet implemented, returning err=%d\n",MOD_NAME,__func__,err);
	return err;
}

static void pci_testdev_remove(struct pci_dev *pdev)
{
	dev_dbg(&pdev->dev, "%s(%d): PTI PCI ID %04x:%04x\n",
			__func__, __LINE__, pdev->vendor, pdev->device);
	pr_err("%s: %s not yet implemented\n",MOD_NAME,__func__);
}

static int pci_driver_reged = 0;
static struct pci_driver pci_testdev_pci_driver = {
        .name           = MOD_NAME,
        .id_table       = pci_ids,
        .probe          = pci_testdev_probe,
        .remove         = pci_testdev_remove,
};


static void pci_testdev_cleanup(void)
{
	if (pci_driver_reged){
		pci_unregister_driver(&pci_testdev_pci_driver);
		pci_driver_reged=0;
		pr_debug("%s: unregistered PCI driver\n",MOD_NAME);
	}
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

	err = pci_register_driver(&pci_testdev_pci_driver);
	if (err) {
		pr_err("%s(%d): PCI registration failed of %s driver\n",
			__func__, __LINE__,MOD_NAME);
		pr_err("%s(%d): Error value returned: %d\n",
			__func__, __LINE__, err);
		goto exit0;
	}
	pci_driver_reged=1;
	pr_debug("%s: registered PCI driver\n",MOD_NAME);
	pr_debug("%s: WARNING! PCI probe is asynchronous! Probe can still fail!\n",MOD_NAME);

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
