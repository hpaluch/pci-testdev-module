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
#include <linux/io.h>

#define MOD_PCI_VID 0x1b36
#define MOD_PCI_DID 0x0005
#define MOD_SYS_CLASS_PATH "/sys/class"
#define MOD_CLASS "pci-testdev"
#define MOD_NAME "pci-testdev"
#define MOD_VER  "0.0.5"


// structure per PCI device
struct pci_testdev_card {
	struct pci_dev *my_pci_dev;
	int card_enabled; // flag for cleanup
	int release_regions; // flag for cleanup
	void __iomem *bar0_vaddr;
	void __iomem *bar1_vaddr;
};

static struct class *pci_testdev_class = NULL;

static const struct pci_device_id pci_ids[] = {
                {PCI_DEVICE(MOD_PCI_VID, MOD_PCI_DID)},
                {0}
};

static void pci_testdev_remove(struct pci_dev *pdev)
{
	struct pci_testdev_card *testdev_card = pci_get_drvdata(pdev);

	pci_dbg(pdev, "%s:%d PCI ID %04x:%04x SSID %04x:%04x  - instance 0x%p device - ENTER\n",
			__func__, __LINE__,
			pdev->vendor, pdev->device,pdev->subsystem_vendor,pdev->subsystem_device,
			testdev_card);

	if (!testdev_card){
		pci_err(pdev,"PCI ID %04x:%04x SSID %04x:%04x - Instance pointer is NULL! - no cleanup possible!\n",
				pdev->vendor, pdev->device,pdev->subsystem_vendor,pdev->subsystem_device);
		return;
	}

	if (testdev_card->bar1_vaddr){
		pci_dbg(pdev,"Unmapping BAR%d from vaddr=%lx\n",1,(unsigned long)testdev_card->bar1_vaddr);
		pci_iounmap(pdev,testdev_card->bar1_vaddr);
		testdev_card->bar1_vaddr = NULL;
	}

	if (testdev_card->bar0_vaddr){
		pci_dbg(pdev,"Unmapping BAR%d from vaddr=%lx\n",0,(unsigned long)testdev_card->bar0_vaddr);
		pci_iounmap(pdev,testdev_card->bar0_vaddr);
		testdev_card->bar0_vaddr = NULL;
	}

	if (testdev_card->release_regions){
		pci_release_regions(pdev);
		pci_dbg(pdev,"cleanup: PCI regions released\n");
		testdev_card->release_regions = 0;
	}

	if (testdev_card->card_enabled){
		pci_disable_device(pdev);
		pci_dbg(pdev,"cleanup: PCI card disabled\n");
		testdev_card->card_enabled = 0;
	}

	pci_dbg(pdev,"cleanup: Freeing testdev_card structure at 0x%p len %d\n",
			testdev_card,(int)sizeof(*testdev_card));
	// invalidate data to crash dangling pointers
	memset(testdev_card,254,sizeof(*testdev_card));
	kfree(testdev_card);
	testdev_card = NULL;
	pci_set_drvdata(pdev, NULL);
	pci_dbg(pdev,"%s:%d - EXIT\n",__func__,__LINE__);
}

static int pci_testdev_probe(struct pci_dev *pdev,
		const struct pci_device_id *ent)
{
	struct pci_testdev_card *testdev_card = NULL;
	int err = -EINVAL;

	dev_dbg(&pdev->dev, "%s:%d PCI ID %04x:%04x SSID %04x:%04x - ENTER\n",
			__func__, __LINE__, pdev->vendor, pdev->device,
			pdev->subsystem_vendor,pdev->subsystem_device);

	testdev_card = kzalloc(sizeof(*testdev_card), GFP_KERNEL);
	if (!testdev_card){
		dev_err(&pdev->dev,"PCI ID %04x:%04x SSID %04x:%04x Unable to alloc %d bytes\n",
				pdev->vendor, pdev->device,pdev->subsystem_vendor,pdev->subsystem_device,
				(int)sizeof(*testdev_card));
		err = -ENOMEM;
		goto exit0;
	}
	testdev_card->my_pci_dev = pdev;
	pci_set_drvdata(pdev, testdev_card);

	err = pci_enable_device(pdev);
	if (err){
		dev_err(&pdev->dev,"pci_enable_device() failed with err=%d for PCI ID %04x:%04x SSID %04x:%04x\n",
				err,pdev->vendor, pdev->device,pdev->subsystem_vendor,pdev->subsystem_device);
		goto exit1;
	}
	testdev_card->card_enabled = 1;
	dev_dbg(&pdev->dev,"OK PCI card enabled\n");

	err = pci_request_regions(pdev, MOD_NAME);
	if (err){
		dev_err(&pdev->dev,"pci_request_regions() failed with err=%d for PCI ID %04x:%04x SSID %04x:%04x\n",
				err,pdev->vendor, pdev->device,pdev->subsystem_vendor,pdev->subsystem_device);
		goto exit1;
	}
	testdev_card->release_regions=1;
	dev_dbg(&pdev->dev,"OK PCI regions requested\n");

	testdev_card->bar0_vaddr = pci_iomap(pdev, 0, 0);
	if (testdev_card->bar0_vaddr==NULL){
		pci_err(pdev,"pci_iomap(BAR%d) failed with err=%d for PCI ID %04x:%04x SSID %04x:%04x\n",
				0,err,pdev->vendor, pdev->device,pdev->subsystem_vendor,pdev->subsystem_device);
		goto exit1;
	}
	pci_dbg(pdev,"BAR%d mapped at vaddr %lx\n",0,(unsigned long)testdev_card->bar0_vaddr);

	testdev_card->bar1_vaddr = pci_iomap(pdev, 1, 0);
	if (testdev_card->bar1_vaddr==NULL){
		pci_err(pdev,"pci_iomap(BAR%d) failed with err=%d for PCI ID %04x:%04x SSID %04x:%04x\n",
				1,err,pdev->vendor, pdev->device,pdev->subsystem_vendor,pdev->subsystem_device);
		goto exit1;
	}
	pci_dbg(pdev,"BAR%d mapped at vaddr %lx\n",1,(unsigned long)testdev_card->bar1_vaddr);

	// TODO: There is also optional BAR2

	err = 0;
	dev_info(&pdev->dev, "OK PCI ID %04x:%04x SSID %04x:%04x adding instance 0x%p\n",
			pdev->vendor, pdev->device,pdev->subsystem_vendor,pdev->subsystem_device,
			testdev_card);
	goto exit0;
exit1:
	dev_err(&pdev->dev,"Calling cleanup for PCI ID %04x:%04x SSID %04x:%04x after err=%d\n",
			pdev->vendor, pdev->device,pdev->subsystem_vendor,pdev->subsystem_device,
			err);
	pci_testdev_remove(pdev);
exit0:
	dev_dbg(&pdev->dev,"%s:%d - EXIT, err=%d\n",__func__, __LINE__,err);
	return err;
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

	pr_debug("%s: %s:%d - ENTER\n",MOD_NAME,__func__,__LINE__);
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
		pr_err("%s: PCI registration failed with err=%d\n",
			MOD_NAME,err);
		goto exit0;
	}
	pci_driver_reged=1;
	pr_debug("%s: registered PCI driver\n",MOD_NAME);
	pr_debug("%s: WARNING! Individual card probe may still fail!\n",MOD_NAME);

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
	pr_debug("%s: %s:%d - EXIT, err=%d\n",MOD_NAME,__func__,__LINE__,err);
	return err;
}
static void __exit pci_testdev_exit(void)
{
	pr_debug("%s: %s:%d - ENTER\n",MOD_NAME,__func__,__LINE__);
	pci_testdev_cleanup();
	pr_info("%s: v%s unloaded\n",MOD_NAME,MOD_VER);
	pr_debug("%s: %s:%d - EXIT\n",MOD_NAME,__func__,__LINE__);
}

module_init(pci_testdev_init);
module_exit(pci_testdev_exit);

MODULE_VERSION(MOD_VER);
MODULE_DESCRIPTION("Tester of QEMU PCI test device");
MODULE_AUTHOR("Henryk Paluch");
MODULE_LICENSE("GPL v2");
