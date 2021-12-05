#ifndef __MAGMA_BROADCOM_H
    #define __MAGMA_BROADCOM_H

    #ifndef __BCM47XX_NVRAM_H
        #include <linux/bcm47xx_nvram.h>
    #endif

    /* status of operations, problably will be unused */
    enum magma_broadcom_state{
        LOAD_SUCCESS,
        LOAD_ERROR,
        WRITE_SUCCESS = 0,
        WRITE_ERROR,    
    };

    /* list of availables I/O operations for the IOCTLs */ 
    enum magma_broadcom_available_io_ops{
        MAGMA_BROADCOM_DO_READ8 = 8,
        MAGMA_BROADCOM_DO_READ32 = 32,
        MAGMA_BROADCOM_DO_WRITE8 = 64,
        MAGMA_BROADCOM_DO_WRITE32 = 128,
        MAGMA_BROADCOM_DO_RAM_WRITE32 = 110,
    }magma_broadcom_available_io_ops;
    
    /* results of the I/O operations, I've decided to create a specific struct which contains all the different return types for the Output Operations */
    struct magma_io_res{
        u8 read8_res;
        u32 read32_res;
        short int write_res;
    };

    /* list of the availables IOCTL offered by this module, note that is reduced cause most of the ieee80211_ops functions are useless for our purposes */
    enum magma_broadcom_available_ioctl{
        MAGMA_BROADCOM_EXECUTE_PCI_READ32 = 1110,
        MAGMA_BROADCOM_EXECUTE_PCI_WRITE32,
        MAGMA_BROADCOM_EXECUTE_DEBUGMODE,
        MAGMA_BROADCOM_GET_FW_NAME,
        MAGMA_BROADCOM_SCAN,
        MAGMA_BROADCOM_START_AP,
        MAGMA_BROADCOM_TX,
    }magma_broadcom_available_ioctl;

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

    enum magma_broadcom_bus_state {
    	BRCMF_BUS_DOWN,		/* Not ready for frame transfers */
    	BRCMF_BUS_UP		/* Ready for frame transfers */
    };
	
    /* states of the SDIO bus, NOMEDIUM indicates that isn't present */
    enum magma_broadcom_sdiod_state {
    	BRCMF_SDIOD_DOWN = 0,
    	BRCMF_SDIOD_DATA, /* now BRCMF_SDIOD_DATA is equal to 1, will be usefull in future :) */
    	BRCMF_SDIOD_NOMEDIUM
    };

    enum magma_broadcom_bus_protocol_type {
    	BRCMF_PROTO_BCDC,
    	BRCMF_PROTO_MSGBUF
    };

    /* set of structures which represent the SDIO bus interface for communicating with the bcmXX 802.11 adapters */

    struct brcmf_bus {
    	union {
    		struct brcmf_sdio_dev *sdio;
    		struct brcmf_usbdev *usb;
    		struct brcmf_pciedev *pcie;
    	} bus_priv;
    	enum magma_broadcom_bus_protocol_type proto_type;
    	struct device *dev;
    	enum magma_broadcom_bus_state state;
    };
    
    /* sdio device */
    struct brcmf_sdio_dev {
       	struct sdio_func *func1;
    	struct brcmf_sdio *bus;
    	struct device *dev;
    	struct brcmf_bus *bus_if;
    #ifndef BRCMF_FW_NAME_LEN
	        #define	BRCMF_FW_NAME_LEN       320
	#endif
	    char fw_name[BRCMF_FW_NAME_LEN];
	    char nvram_name[BRCMF_FW_NAME_LEN];
	    enum magma_broadcom_sdiod_state state;
    };

    struct magma_broadcomm_sdio_bus{
	    union {
	    	struct brcmf_sdio_dev *sdio;
	    	struct brcmf_usbdev *usb;
	    	struct brcmf_pciedev *pcie;
	    } bus_priv;
	    enum magma_broadcom_bus_protocol_type proto_type;
	    struct device *dev;
	    enum magma_broadcom_bus_state state;
    }magma_broadcomm_sdio_bus;

    /* counters for the commands in the SDIO bus, used for the bcm43XX chips */
    struct brcmf_sdio_count {
	    ulong tx_ctlerrs;	/* Err of sending ctrl frames */
	    ulong tx_ctlpkts;	/* Ctrl frames sent to dongle */
	    ulong rx_ctlerrs;	/* Err of processing rx ctrl frames */
	    ulong rx_ctlpkts;	/* Ctrl frames processed from dongle */
	    ulong rx_readahead_cnt;	/* packets where header read-ahead was used */
    };
    
    struct brcmf_sdio {
        struct brcmf_sdio_dev *sdiodev;
	    bool intr;		/* Use interrupts */
	    bool poll;		/* Use polling */
	    atomic_t ipend;		/* Device interrupt is pending */
#ifdef DEBUG
	    struct brcmf_console console;	/* Console output polling support */
#endif
	    u8 *ctrl_frame_buf;
	    u16 ctrl_frame_len;
	    bool ctrl_frame_stat;
	    int ctrl_frame_err;
	    struct workqueue_struct *brcmf_wq;
	    struct work_struct datawork;
	    bool dpc_triggered;
	    struct brcmf_sdio_count sdcnt;
    };

    /* function prototypes */
    static int magma_broadcom_request_fw(const struct firmware **fw, struct device *magma_bcm_dev);
    static void magma_broadcom_pci_write8(void __iomem *magma_broadcom_pci_mmio, u16 offset, u8 value_to_write);
    static u8 magma_broadcom_pci_read8(void __iomem *magma_broadcom_pci_mmio, u16 offset);
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
        if( request_firmware(fw, MAGMA_BROADCOM_GET_FW_NAME, magma_bcm_dev) == 0 ){
            return LOAD_SUCCESS;
        }else{
            return LOAD_ERROR;
        }
    }

    /*
        the following functions are for the Broadcom PCI Wlan cards, note that are minimalistic enough for educative purposes
        magma_broadcom_pci_write8
        magma_broadcom_pci_write32
        magma_broadcom_pci_read8
        magma_broadcom_pci_read32
        magma_broadcom_ram_write32

    */

    /* magma_broadcom_pci_write8 */
    static void magma_broadcom_pci_write8(void __iomem *magma_broadcom_pci_mmio, u16 offset, u8 value_to_write){
        #ifndef BCMA_CORE_SIZE
              #define BCMA_CORE_SIZE 0x1000
        #endif
        offset += (2 * BCMA_CORE_SIZE);
        #ifndef __GENERIC_IO_H
            #include <asm-generic/iomap.h>
        #endif
        return iowrite8(value_to_write, (offset + magma_broadcom_pci_mmio));

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
        iowrite32(value_to_write, (offset + magma_broadcom_pci_mmio));
        return 0;
        }

    /* magma_broadcom_pci_read8 */
    static u8 magma_broadcom_pci_read8(void __iomem *magma_broadcom_pci_mmio, u16 offset){
       #ifndef BCMA_CORE_SIZE
              #define BCMA_CORE_SIZE 0x1000
        #endif
        offset += (2 * BCMA_CORE_SIZE);
        #ifndef __GENERIC_IO_H
            #include <asm-generic/iomap.h>
        #endif
	    return ioread8(magma_broadcom_pci_mmio + offset);
    }

    /* magma_broadcom_pci_read32 */
    static u32 magma_broadcom_pci_read32(void __iomem *magma_broadcom_pci_mmio, u16 offset){
        #ifndef BCMA_CORE_SIZE
              #define BCMA_CORE_SIZE 0x1000
        #endif
        offset += (2 * BCMA_CORE_SIZE);
        #ifndef __GENERIC_IO_H
            #include <asm-generic/iomap.h>
        #endif
        return ioread32(magma_broadcom_pci_mmio + offset);
        }

    /* write 32 bits directly into the wlan chip RAM, this function
        has been taken from here https://elixir.bootlin.com/linux/v4.14/source/drivers/net/wireless/broadcom/b43/main.c#L477 */
    static void magma_broadcom_ram_write32(void __iomem *magma_broadcom_pci_mmio, u16 offset, u32 val){
        u32 mac_ctrl;
        #ifndef B43_MMIO_MACCTL
            #define B43_MMIO_MACCTL			0x120
        #endif
        mac_ctrl = magma_broadcom_pci_read32(magma_broadcom_pci_mmio, B43_MMIO_MACCTL);
        if (mac_ctrl & 0x00010000){ /* where '0x00010000' is referred to the Big Endian mode */
            #ifndef _LINUX_SWAB_H
                #include <linux/swab.h>
            #endif
		    val = swab32(val);
        }
        #ifndef B43_MMIO_RAM_CONTROL
            #define B43_MMIO_RAM_CONTROL		0x130
        #endif
	    magma_broadcom_pci_write32(magma_broadcom_pci_mmio, B43_MMIO_RAM_CONTROL, offset);
        #ifndef B43_MMIO_RAM_DATA
            #define B43_MMIO_RAM_DATA		0x134
        #endif
	    magma_broadcom_pci_write32(magma_broadcom_pci_mmio, B43_MMIO_RAM_DATA, val);
    }

    #ifndef _LINUX_TYPES_H
        #include <linux/types.h>
    #endif

    /* this is the main I/O function, the idea behind that is that this would be the backend for the IOCTLing system adopted by the Magma driver */
    static struct magma_io_res *magma_broadcom_main_io(resource_size_t bus_addr, enum magma_broadcom_available_io_ops ops_code, u16 offset, u32 value_to_write){
        struct magma_io_res *magma_io_struct = (struct magma_io_res *)kmalloc(sizeof(magma_io_struct), GFP_USER);        
        void __iomem *memory_area;
        magma_io_struct->read8_res = 0;
        magma_io_struct->read32_res = 0;
        #ifndef BCMA_CORE_SIZE
            #define BCMA_CORE_SIZE 0x1000
        #endif
        memory_area = ioremap(bus_addr, BCMA_CORE_SIZE);
        switch(magma_broadcom_available_io_ops){
            case MAGMA_BROADCOM_DO_READ8:
                if( !!( magma_io_struct->read8_res = magma_broadcom_pci_read8(memory_area, offset) ) ){
                    return magma_io_struct;
                }else{
                    magma_io_struct->read8_res = 0;
                    return magma_io_struct;
                }
            case MAGMA_BROADCOM_DO_READ32:
                if( !!( magma_io_struct->read32_res = magma_broadcom_pci_read32(memory_area, offset) ) ){
                    return magma_io_struct;
                }else{
                    magma_io_struct->read32_res = 0;
                    return magma_io_struct;
                }
            case MAGMA_BROADCOM_DO_WRITE8:
                magma_broadcom_pci_write8(memory_area, offset, value_to_write);
                magma_io_struct->write_res = 1;
                return magma_io_struct;
            case MAGMA_BROADCOM_DO_WRITE32:
                magma_broadcom_pci_write32(memory_area, offset, value_to_write);
                magma_io_struct->write_res = 1;
                return magma_io_struct;
            case MAGMA_BROADCOM_DO_RAM_WRITE32:
                magma_broadcom_ram_write32(memory_area, offset, value_to_write);
                magma_io_struct->write_res = 1;
                return magma_io_struct;
            default:
                return magma_io_struct;
        }
    }

EXPORT_SYMBOL(magma_broadcom_main_io);

    /* IOCTL processing part, calls 'magma_broadcom_main_io' for backend I/O operations */
    static long magma_broadcom_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
        switch(cmd){


            default:
            #ifndef SUPPORTED_IOCTL_NOT_FOUND
                #define SUPPORTED_IOCTL_NOT_FOUND 55
            #endif
            return SUPPORTED_IOCTL_NOT_FOUND;
        }
    }
#endif
