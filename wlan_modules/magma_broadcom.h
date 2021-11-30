#ifndef __MAGMA_BROADCOM_H
    #define __MAGMA_BROADCOM_H

    #ifndef __BCM47XX_NVRAM_H
        #include <linux/bcm47xx_nvram.h>
    #endif

    enum magma_broadcom_state{
        LOAD_SUCCESS,
        LOAD_ERROR,
        WRITE_SUCCESS = 0,
        WRITE_ERROR,    
    };
    /* this enum represent all commands supported by the brcmfmac driver */

    enum magma_broadcom_hcmd{
        BRCMF_C_GET_VERSION = 1,
        BRCMF_C_UP = 2,
        BRCMF_C_DOWN = 3,
        BRCMF_C_SET_PROMISC = 10,
        BRCMF_C_GET_RATE = 12,
        BRCMF_C_GET_INFRA = 19,
        BRCMF_C_SET_INFRA = 20,
        BRCMF_C_GET_AUTH = 21,
        BRCMF_C_SET_AUTH = 22,
        BRCMF_C_GET_BSSID = 23,
        BRCMF_C_GET_SSID = 25,
        BRCMF_C_SET_SSID = 26,
        BRCMF_C_TERMINATED = 28,
        BRCMF_C_GET_CHANNEL = 29,
        BRCMF_C_SET_CHANNEL = 30,
        BRCMF_C_GET_SRL = 31,
        BRCMF_C_SET_SRL = 32,
        BRCMF_C_GET_LRL = 33,
        BRCMF_C_SET_LRL = 34,
        BRCMF_C_GET_RADIO = 37,
        BRCMF_C_SET_RADIO = 38,
        BRCMF_C_GET_PHYTYPE = 39,
        BRCMF_C_SET_KEY = 45,
        BRCMF_C_GET_REGULATORY = 46,
        BRCMF_C_SET_REGULATORY = 47,
        BRCMF_C_SET_PASSIVE_SCAN = 49,
        BRCMF_C_SCAN = 50,
        BRCMF_C_SCAN_RESULTS = 51,
        BRCMF_C_DISASSOC = 52,
        BRCMF_C_REASSOC = 53,
        BRCMF_C_SET_ROAM_TRIGGER = 55,
        BRCMF_C_SET_ROAM_DELTA = 57,
        BRCMF_C_GET_BCNPRD = 75,
        BRCMF_C_SET_BCNPRD = 76,
        BRCMF_C_GET_DTIMPRD = 77,
        BRCMF_C_SET_DTIMPRD = 78,
        BRCMF_C_SET_COUNTRY = 84,
        BRCMF_C_GET_PM = 85,
        BRCMF_C_SET_PM = 86,
        BRCMF_C_GET_REVINFO = 98,
        BRCMF_C_GET_MONITOR = 107,
        BRCMF_C_SET_MONITOR = 108,
        BRCMF_C_GET_CURR_RATESET = 114,
        BRCMF_C_GET_AP = 117,
        BRCMF_C_SET_AP = 118,
        BRCMF_C_SET_SCB_AUTHORIZE = 121,
        BRCMF_C_SET_SCB_DEAUTHORIZE = 122,
        BRCMF_C_GET_RSSI = 127,
        BRCMF_C_GET_WSEC = 133,
        BRCMF_C_SET_WSEC = 134,
        BRCMF_C_GET_PHY_NOISE = 135,
        BRCMF_C_GET_BSS_INFO = 136,
        BRCMF_C_GET_GET_PKTCNTS = 137,
        BRCMF_C_GET_BANDLIST = 140,
        BRCMF_C_SET_SCB_TIMEOUT = 158,
        BRCMF_C_GET_ASSOCLIST = 159,
        BRCMF_C_GET_PHYLIST = 180,
        BRCMF_C_SET_SCAN_CHANNEL_TIME = 185,
        BRCMF_C_SET_SCAN_UNASSOC_TIME = 187,
        BRCMF_C_SCB_DEAUTHENTICATE_FOR_REASON = 201,
        BRCMF_C_SET_ASSOC_PREFER = 205,
        BRCMF_C_GET_VALID_CHANNELS = 217,
        BRCMF_C_SET_FAKEFRAG = 219,
        BRCMF_C_GET_KEY_PRIMARY = 235,
        BRCMF_C_SET_KEY_PRIMARY = 236,
        BRCMF_C_SET_SCAN_PASSIVE_TIME = 258,
        BRCMF_C_GET_VAR = 262,
        BRCMF_C_SET_VAR = 263,
        BRCMF_C_SET_WSEC_PMK = 268,
        };

    static int magma_broadcom_request_fw(const struct firmware **fw, struct device *magma_bcm_dev);
    static int magma_broadcom_pci_write32(void __iomem *magma_broadcom_pci_mmio, u16 offset, u32 value_to_write);
    static u32 magma_broadcom_pci_read32(void __iomem *magma_broadcom_pci_mmio, u16 offset);
    static int magma_sdio_host_claimer(struct mmc_host *host, struct mmc_ctx *ctx, atomic_t *abort);
    static int magma_broadcom_send_sdio_hcmd(enum magma_broadcom_hcmd msg);

    /* this is a modified version of the Linux's kernel mmc_claim_host function, used for claiming the SDIO host for initializing a set of operations on it */
    static int magma_sdio_host_claimer(struct mmc_host *host, struct mmc_ctx *ctx, atomic_t *abort){
	    struct task_struct *task = ctx ? NULL : current;
	    DECLARE_WAITQUEUE(wait, current);
	    unsigned long flags;
	    int stop;
	    bool pm = false;
	    might_sleep();
	    add_wait_queue(&host->wq, &wait);
	    spin_lock_irqsave(&host->lock, flags);
	    while (1) {
		    set_current_state(TASK_UNINTERRUPTIBLE);
		    stop = abort ? atomic_read(abort) : 0;
		    if( stop || !host->claimed || (host->claimer == ctx || (!ctx && task && host->claimer->task == task)) ){
			    break;
            }
		    spin_unlock_irqrestore(&host->lock, flags);
		    schedule();
		    spin_lock_irqsave(&host->lock, flags);
	    }
	    set_current_state(TASK_RUNNING);
	    if (!stop) {
		    host->claimed = 1;
    	if(!host->claimer) {
		    if(ctx){
			    host->claimer = ctx;
		}else{
			host->claimer = &host->default_ctx;
	        }
	    }
	    if (task){
		    host->claimer->task = task;
        }
		host->claim_cnt += 1;
		if (host->claim_cnt == 1)
			pm = true;
	    }else
            wake_up(&host->wq);
	    spin_unlock_irqrestore(&host->lock, flags);
	    remove_wait_queue(&host->wq, &wait);
    if(pm){
        #ifdef CONFIG_PM
        #ifndef RPM_GET_PUT
            #define RPM_GET_PUT		0x04
        #endif
		    __pm_runtime_resume(host->parent, RPM_GET_PUT);
        #endif
    }
	return stop;
    }

    /* main function used for sending Host CoMmanDs for broadcomm devices using SDIO bus */
    static int magma_broadcom_send_sdio_hcmd(enum magma_broadcom_hcmd msg){
        wait_queue_head_t wireless_wait_queue;
        spinlock_t wifi_spinlock;
	    int ret;
        int msg_len;
        u8 *cmd;
        struct mmc_host wireless_host_iface;
        struct mmc_card *wifi_card = (struct mmc_card *)kmalloc(sizeof(wifi_card), GFP_KERNEL);
        struct sdio_func *wireless_sdio = (struct sdio_func *)kmalloc(sizeof(wireless_sdio), GFP_KERNEL);
        struct brcmf_sdio_dev sdio_complementary;
        struct brcmf_sdio *sdio_complementary_bus = (struct brcmf_sdio *)kmalloc(sizeof(sdio_complementary_bus), GFP_KERNEL);
        struct brcmf_bus *bus_iface = (struct brcmf_bus *)kmalloc(sizeof(struct brcmf_bus), GFP_KERNEL); // our struct to fill
        struct task_struct *wifi_task = current;
        struct mmc_ctx *wifi_claimer = (struct mmc_ctx *)kmalloc(sizeof(wifi_claimer), GFP_KERNEL);
	    struct brcmf_sdio_dev *sdiodev = bus_iface->bus_priv.sdio;
	    struct brcmf_sdio *bus = sdiodev->bus; // HERE THERE ARE THE PROBLEMS
        wifi_claimer->task = wifi_task;
        spin_lock_init(&wifi_spinlock);
        init_waitqueue_head(&wireless_wait_queue);
        // set up the complementary bus, this will then added to the bus field in the sdio_complementary
        sdio_complementary_bus->sdcnt.tx_ctlerrs = 0; // set the error ctrl frames to 0 
        sdio_complementary_bus->sdcnt.tx_ctlpkts = 0; // set the successfull ctrl frames to 0
        sdio_complementary_bus->sdiodev = &sdio_complementary;
        //fill before wireless_sdio
        if( sizeof(wifi_spinlock) != 0 ){
                //wireless_wait_queue.lock = wifi_spinlock;
        }
        wireless_host_iface.wq = wireless_wait_queue;
        wireless_host_iface.lock = wifi_spinlock;
        wireless_host_iface.claimed = 1;
        wireless_host_iface.claimer = wifi_claimer;
        wireless_host_iface.claim_cnt = 0;
        wifi_card->host = &wireless_host_iface;
        wireless_sdio->card = wifi_card;
        wireless_sdio->enable_timeout = 1;
        wireless_sdio->num_info = 0;
        sdio_complementary_bus->sdiodev->func1 = wireless_sdio;
        sdio_complementary_bus->dpc_triggered = true;
        sdio_complementary.state = BRCMF_SDIOD_DATA;
        sdio_complementary.bus = sdio_complementary_bus;
        //sdio_complementary.bus = sdio_complementary_bus;
        // fill the brcmf_bus structure
        bus_iface->bus_priv.sdio = &sdio_complementary;

        cmd = cpu_to_le32(msg);
        msg_len = cpu_to_le32( sizeof(msg) );
	    /* Send from dpc */
	    bus->ctrl_frame_buf = cmd; 
	    bus->ctrl_frame_len = msg_len;
	    wmb();
	    bus->ctrl_frame_stat = true;
        if(!bus->dpc_triggered) {
		    bus->dpc_triggered = true;
		    queue_work(bus->brcmf_wq, &bus->datawork);
	    }
	    ret = 0;
	    if(bus->ctrl_frame_stat) {
		    sdio_claim_host(bus->sdiodev->func1);
		    magma_sdio_host_claimer(bus->sdiodev->func1->card->host, NULL, NULL);
		if(bus->ctrl_frame_stat) {
			bus->ctrl_frame_stat = false;
			ret = -ETIMEDOUT;
		}
		sdio_release_host(bus->sdiodev->func1);
	    }
	    if(!ret) {
		    rmb();
		    ret = bus->ctrl_frame_err;
	    }
	    if(ret){
		    bus->sdcnt.tx_ctlerrs++;
	    }else{
		    bus->sdcnt.tx_ctlpkts++;
        }
        kfree(wifi_card);
        kfree(wireless_sdio);
        kfree(sdio_complementary_bus);
        kfree(bus_iface);
        kfree(wifi_claimer);
	    return ret;
}

EXPORT_SYMBOL(magma_broadcom_send_sdio_hcmd);


    /* firmware load part, from: https://elixir.bootlin.com/linux/latest/source/drivers/net/wireless/broadcom/brcm80211/brcmfmac/firmware.c#L615 */
    static int magma_broadcom_request_fw(const struct firmware **fw, struct device *magma_bcm_dev){
        if( request_firmware(fw, MAGMA_BROADCOM_FW_NAME, magma_bcm_dev) == 0 ){
            return LOAD_SUCCESS;
        }else{
            return LOAD_ERROR;
        }
    }

    static int magma_broadcom_unload_fw(){

        return 0;
    }
    /* magma_broadcom_pci_write32*/ 
    static int magma_broadcom_pci_write32(void __iomem *magma_broadcom_pci_mmio, u16 offset, u32 value_to_write){
        #ifndef BCMA_CORE_SIZE
              #define BCMA_CORE_SIZE 0x1000
        #endif
        offset += (2 * BCMA_CORE_SIZE);
        #ifndef __GENERIC_IO_H
            #include <asm-generic/iomap.h>
        #endif
        if( iowrite32(value_to_write, (offset + magma_broadcom_pci_mmio)) ){
            return WRITE_SUCCESS;
        }else{
            return WRITE_ERROR;
        }

    static u32 magma_broadcom_pci_read32(void __iomem *magma_broadcom_pci_mmio, u16 offset){
        #ifndef BCMA_CORE_SIZE
              #define BCMA_CORE_SIZE 0x1000
        #endif
        offset += (2 * BCMA_CORE_SIZE);
        #ifndef __GENERIC_IO_H
            #include <asm-generic/iomap.h>
        #endif
        return ioread32(mmio + offset);
        }
}
#endif
