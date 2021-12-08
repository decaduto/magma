/*  © 2021 Edoardo Mantovani All Rights Reserved */

/* Note that this driver has been declared obsolete since Linux Kernel version 2.6.X, but for educative purposes, I've decided to implement it, this because is the simplest alternative driver we can implement for now :)
    - source code: https://elixir.bootlin.com/linux/2.4.30/source/drivers/net/wireless/airo.c
*/

#ifndef __MAGMA_AIRO_H
    #define __MAGMA_AIRO_H

    /* note that this driver did not use cfg80211 nor ieee80211 '_ops' structures, it handles the wlan device in the Minix Style, so it uses only the pci library for interact with it */

    /* PCI vendor/product ID couples */        
    static struct pci_device_id card_ids[] = {
	    { 0x14b9, 1, PCI_ANY_ID, PCI_ANY_ID, },
	    { 0x14b9, 0x4500, PCI_ANY_ID, PCI_ANY_ID },
	    { 0x14b9, 0x4800, PCI_ANY_ID, PCI_ANY_ID, },
	    { 0x14b9, 0x0340, PCI_ANY_ID, PCI_ANY_ID, },
	    { 0x14b9, 0x0350, PCI_ANY_ID, PCI_ANY_ID, },
	    { 0, }
    };
    MODULE_DEVICE_TABLE(pci, card_ids);


    static int magma_airo_pci_probe(struct pci_dev *airo_dev, const struct pci_device_id *pent);
    static int magma_airo_xmit_frame();

    inline static int magma_airo_pci_probe(struct pci_dev *airo_dev, const struct pci_device_id *pent){
	    struct net_device *dev;

	    if (pci_enable_device(airo_dev)){
		    return -ENODEV;
        }
	    pci_set_master(airo_dev);

	    dev = init_airo_card(airo_dev->irq,	airo_dev->resource[2].start, 0);
	    if (!dev){
		    return -ENODEV;
        }
	    pci_set_drvdata(airo_dev, dev);
	    set_bit (FLAG_PCI, &((struct airo_info *)airo_dev->priv)->flags);
	    return 0;
    }

    inline static int magma_airo_xmit_frame(){
        return 0;
    }
#endif

