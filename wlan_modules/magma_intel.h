#ifndef __MAGMA_INTEL_H
    #define __MAGMA_INTEL_H

enum magma_iwlwifi_read_len{
    MAGMA_IWLWIFI_WRITE_BYTE = 8,
    MAGMA_IWLWIFI_WRITE_LONG = 32,
    MAGMA_IWLWIFI_READ_LONG = 64,
    MAGMA_IWLWIFI_READ_CONFIG_LONG = 128,
};

/* the pci_device_id struct contains all PCI devices Vendor / Product IDs, will be used during the probe function invocation in a for loop for determining if the host has attached a PCI device which is conform */
static struct pci_device_id magm_supported_pci[] = {
    { PCI_DEVICE(0x2526, 0x1551) },
    { PCI_DEVICE(0x2526, 0x1550) },
    { PCI_DEVICE(0x2526, 0x1551) },
    { PCI_DEVICE(0x2526, 0x1552) },
    { PCI_DEVICE(0x30DC, 0x1551) },
    { PCI_DEVICE(0x30DC, 0x1552) },
    { PCI_DEVICE(0x31DC, 0x1551) },
    { PCI_DEVICE(0x31DC, 0x1552) },
    { PCI_DEVICE(0xA370, 0x1551) },
    { PCI_DEVICE(0xA370, 0x1552) },
    { PCI_DEVICE(0x51F0, 0x1552) },
    { PCI_DEVICE(0x51F0, 0x1551) },
    { PCI_DEVICE(0x271C, 0x0214) },
    { PCI_DEVICE(0x2723, 0x1653) },
    { PCI_DEVICE(0x2723, 0x1654) },
    { PCI_DEVICE(0x43F0, 0x0070) },
    { PCI_DEVICE(0x43F0, 0x0074) },
    { PCI_DEVICE(0x43F0, 0x0078) },
    { PCI_DEVICE(0x43F0, 0x007C) },
    { PCI_DEVICE(0x43F0, 0x1651) },
    { PCI_DEVICE(0x43F0, 0x1652) },
    { PCI_DEVICE(0x43F0, 0x2074) },
    { PCI_DEVICE(0x43F0, 0x4070) },
    { PCI_DEVICE(0xA0F0, 0x0070) },
    { PCI_DEVICE(0xA0F0, 0x0074) },
    { PCI_DEVICE(0xA0F0, 0x0078) },
    { PCI_DEVICE(0xA0F0, 0x007C) },
    { PCI_DEVICE(0xA0F0, 0x0A10) },
    { PCI_DEVICE(0xA0F0, 0x1651) },
    { PCI_DEVICE(0xA0F0, 0x1652) },
    { PCI_DEVICE(0xA0F0, 0x2074) },
    { PCI_DEVICE(0xA0F0, 0x4070) },
    { PCI_DEVICE(0xA0F0, 0x6074) },
    { PCI_DEVICE(0x02F0, 0x0070) },
    { PCI_DEVICE(0x02F0, 0x0074) },
    { PCI_DEVICE(0x02F0, 0x6074) },
    { PCI_DEVICE(0x02F0, 0x0078) },
    { PCI_DEVICE(0x02F0, 0x007C) },
    { PCI_DEVICE(0x02F0, 0x0310) },
    { PCI_DEVICE(0x02F0, 0x1651) },
    { PCI_DEVICE(0x02F0, 0x1652) },
    { PCI_DEVICE(0x02F0, 0x2074) },
    { PCI_DEVICE(0x02F0, 0x4070) },
    { PCI_DEVICE(0x06F0, 0x0070) },
    { PCI_DEVICE(0x06F0, 0x0074) },
    { PCI_DEVICE(0x06F0, 0x0078) },
    { PCI_DEVICE(0x06F0, 0x007C) },
    { PCI_DEVICE(0x06F0, 0x0310) },
    { PCI_DEVICE(0x06F0, 0x1651) },
    { PCI_DEVICE(0x06F0, 0x1652) },
    { PCI_DEVICE(0x06F0, 0x2074) },
    { PCI_DEVICE(0x06F0, 0x4070) },
    { PCI_DEVICE(0x34F0, 0x0070) },
    { PCI_DEVICE(0x34F0, 0x0074) },
    { PCI_DEVICE(0x34F0, 0x0078) },
    { PCI_DEVICE(0x34F0, 0x007C) },
    { PCI_DEVICE(0x34F0, 0x0310) },
    { PCI_DEVICE(0x34F0, 0x1651) },
    { PCI_DEVICE(0x34F0, 0x1652) },
    { PCI_DEVICE(0x34F0, 0x2074) },
    { PCI_DEVICE(0x34F0, 0x4070) },
    { PCI_DEVICE(0x3DF0, 0x0070) },
    { PCI_DEVICE(0x3DF0, 0x0074) },
    { PCI_DEVICE(0x3DF0, 0x0078) },
    { PCI_DEVICE(0x3DF0, 0x007C) },
    { PCI_DEVICE(0x3DF0, 0x0310) },
    { PCI_DEVICE(0x3DF0, 0x1651) },
    { PCI_DEVICE(0x3DF0, 0x1652) },
    { PCI_DEVICE(0x3DF0, 0x2074) },
    { PCI_DEVICE(0x3DF0, 0x4070) },
    { PCI_DEVICE(0x4DF0, 0x0070) },
    { PCI_DEVICE(0x4DF0, 0x0074) },
    { PCI_DEVICE(0x4DF0, 0x0078) },
    { PCI_DEVICE(0x4DF0, 0x007C) },
    { PCI_DEVICE(0x4DF0, 0x0310) },
    { PCI_DEVICE(0x4DF0, 0x1651) },
    { PCI_DEVICE(0x4DF0, 0x1652) },
    { PCI_DEVICE(0x4DF0, 0x2074) },
    { PCI_DEVICE(0x4DF0, 0x4070) },
    { PCI_DEVICE(0x4DF0, 0x6074) },
    { PCI_DEVICE(0x2725, 0x0090) },
    { PCI_DEVICE(0x2725, 0x0020) },
    { PCI_DEVICE(0x2725, 0x2020) },
    { PCI_DEVICE(0x2725, 0x0024) },
    { PCI_DEVICE(0x2725, 0x0310) },
    { PCI_DEVICE(0x2725, 0x0510) },
    { PCI_DEVICE(0x2725, 0x0A10) },
    { PCI_DEVICE(0x2725, 0xE020) },
    { PCI_DEVICE(0x2725, 0xE024) },
    { PCI_DEVICE(0x2725, 0x4020) },
    { PCI_DEVICE(0x2725, 0x6020) },
    { PCI_DEVICE(0x2725, 0x6024) },
    { PCI_DEVICE(0x2725, 0x1673) },
    { PCI_DEVICE(0x2725, 0x1674) },
    { PCI_DEVICE(0x7A70, 0x0090) },
    { PCI_DEVICE(0x7A70, 0x0098) },
    { PCI_DEVICE(0x7A70, 0x00B0) },
    { PCI_DEVICE(0x7A70, 0x0310) },
    { PCI_DEVICE(0x7A70, 0x0510) },
    { PCI_DEVICE(0x7A70, 0x0A10) },
    { PCI_DEVICE(0x7AF0, 0x0090) },
    { PCI_DEVICE(0x7AF0, 0x0098) },
    { PCI_DEVICE(0x7AF0, 0x00B0) },
    { PCI_DEVICE(0x7AF0, 0x0310) },
    { PCI_DEVICE(0x7AF0, 0x0510) },
    { PCI_DEVICE(0x7AF0, 0x0A10) },
    { PCI_DEVICE(0x2725, 0x00B0) },
    { PCI_DEVICE(0x2726, 0x0090) },
    { PCI_DEVICE(0x2726, 0x0098) },
    { PCI_DEVICE(0x2726, 0x00B0) },
    { PCI_DEVICE(0x2726, 0x00B4) },
    { PCI_DEVICE(0x2726, 0x0510) },
    { PCI_DEVICE(0x2726, 0x1651) },
    { PCI_DEVICE(0x2726, 0x1652) },
    /* ends here the iwlwifi device/product ID */
    {0,}
};

MODULE_DEVICE_TABLE(pci, magm_supported_pci);


/* this enum represent all commands supported by the iwlwifi driver, the enum has been taken from here: https://elixir.bootlin.com/linux/latest/source/drivers/net/wireless/intel/iwlwifi/fw/api/commands.h#L49 */

enum magma_iwlwifi_hcmd {
	UCODE_ALIVE_NTFY = 0x1,
	REPLY_ERROR = 0x2,
	ECHO_CMD = 0x3,
	INIT_COMPLETE_NOTIF = 0x4,
	PHY_CONTEXT_CMD = 0x8,
	DBG_CFG = 0x9,
	SCAN_ITERATION_COMPLETE_UMAC = 0xb5,
	SCAN_CFG_CMD = 0xc,
	SCAN_REQ_UMAC = 0xd,
	SCAN_ABORT_UMAC = 0xe,
	SCAN_COMPLETE_UMAC = 0xf,
	BA_WINDOW_STATUS_NOTIFICATION_ID = 0x13,
	ADD_STA_KEY = 0x17,
	ADD_STA = 0x18,
	REMOVE_STA = 0x19,
	FW_GET_ITEM_CMD = 0x1a,
	TX_CMD = 0x1c,
	TXPATH_FLUSH = 0x1e,
	MGMT_MCAST_KEY = 0x1f,
	SCD_QUEUE_CFG = 0x1d,
	WEP_KEY = 0x20,
	SHARED_MEM_CFG = 0x25,
	TDLS_CHANNEL_SWITCH_CMD = 0x27,
	TDLS_CHANNEL_SWITCH_NOTIFICATION = 0xaa,
	TDLS_CONFIG_CMD = 0xa7,
	MAC_CONTEXT_CMD = 0x28,
	TIME_EVENT_CMD = 0x29,
	TIME_EVENT_NOTIFICATION = 0x2a,
	BINDING_CONTEXT_CMD = 0x2b,
	TIME_QUOTA_CMD = 0x2c,
	NON_QOS_TX_COUNTER_CMD = 0x2d,
	LEDS_CMD = 0x48,
	LQ_CMD = 0x4e,
	FW_PAGING_BLOCK_CMD = 0x4f,
	SCAN_OFFLOAD_REQUEST_CMD = 0x51,
	SCAN_OFFLOAD_ABORT_CMD = 0x52,
	HOT_SPOT_CMD = 0x53,
	SCAN_OFFLOAD_COMPLETE = 0x6D,
	SCAN_OFFLOAD_UPDATE_PROFILES_CMD = 0x6E,
	MATCH_FOUND_NOTIFICATION = 0xd9,
	SCAN_ITERATION_COMPLETE = 0xe7,
	PHY_CONFIGURATION_CMD = 0x6a,
	CALIB_RES_NOTIF_PHY_DB = 0x6b,
	PHY_DB_CMD = 0x6c,
	POWER_TABLE_CMD = 0x77,
	PSM_UAPSD_AP_MISBEHAVING_NOTIFICATION = 0x78,
	LTR_CONFIG = 0xee,
	REPLY_THERMAL_MNG_BACKOFF = 0x7e,
	DC2DC_CONFIG_CMD = 0x83,
	NVM_ACCESS_CMD = 0x88,
	BEACON_NOTIFICATION = 0x90,
	BEACON_TEMPLATE_CMD = 0x91,
	TX_ANT_CONFIGURATION_CMD = 0x98,
	STATISTICS_CMD = 0x9c,
	STATISTICS_NOTIFICATION = 0x9d,
	EOSP_NOTIFICATION = 0x9e,
	REDUCE_TX_POWER_CMD = 0x9f,
	CARD_STATE_NOTIFICATION = 0xa1,
	MISSED_BEACONS_NOTIFICATION = 0xa2,
	MAC_PM_POWER_TABLE = 0xa9,
	MFUART_LOAD_NOTIFICATION = 0xb1,
	RSS_CONFIG_CMD = 0xb3,
	REPLY_RX_PHY_CMD = 0xc0,
	REPLY_RX_MPDU_CMD = 0xc1,
	BAR_FRAME_RELEASE = 0xc2,
	FRAME_RELEASE = 0xc3,
	BA_NOTIF = 0xc5,
	MCC_UPDATE_CMD = 0xc8,
	MCC_CHUB_UPDATE_CMD = 0xc9,
	MARKER_CMD = 0xcb,
	BT_PROFILE_NOTIFICATION = 0xce,
	BT_CONFIG = 0x9b,
	BT_COEX_UPDATE_REDUCED_TXP = 0x5c,
	BT_COEX_CI = 0x5d,
	REPLY_SF_CFG_CMD = 0xd1,
	REPLY_BEACON_FILTERING_CMD = 0xd2,
	DTS_MEASUREMENT_NOTIFICATION = 0xdd,
	LDBG_CONFIG_CMD = 0xf6,
	DEBUG_LOG_MSG = 0xf7,
	BCAST_FILTER_CMD = 0xcf,
	MCAST_FILTER_CMD = 0xd0,
	D3_CONFIG_CMD = 0xd3, /* D3 is the firmware adibited for the WoWlan features, must be studied carefully under the security point of view */
	PROT_OFFLOAD_CONFIG_CMD = 0xd4,
	OFFLOADS_QUERY_CMD = 0xd5,
	D0I3_END_CMD = 0xed,
	WOWLAN_PATTERNS = 0xe0,
	WOWLAN_CONFIGURATION = 0xe1,
	WOWLAN_TSC_RSC_PARAM = 0xe2,
	WOWLAN_TKIP_PARAM = 0xe3,
	WOWLAN_KEK_KCK_MATERIAL = 0xe4,
	WOWLAN_GET_STATUSES = 0xe5,
	SCAN_OFFLOAD_PROFILES_QUERY_CMD = 0x56,
};

/* function prototypes */
static int magma_iwlwifi_spawn_hw_base(struct pci_dev *magma_iwlwifi_pci, void __iomem *hw_base);
static void magma_iwlwifi_pcie_write8(void __iomem *hw_base, u32 offset, u8 value_to_write);
static void magma_iwlwifi_pcie_write32(void __iomem *hw_base, u32 offset, u8 value_to_write);
static u32 magma_iwlwifi_pcie_read32(void __iomem *hw_base, u32 offset);
static void magma_iwlwifi_pcie_set_bitmask(void __iomem *hw_base, u32 register_to_set, u32 mask, u32 value_to_write);
static void magma_iwlwifi_pcie_software_reset(void __iomem *hw_base);
static int magma_iwlwifi_pcie_read_config32(struct pci_dev *magma_pci_dev, u32 offset, u32 *value);
static void magma_iwlwifi_pcie_write_bytes(void *hw_base, enum magma_iwlwifi_read_len magma_iwlwifi_readlength, u32 offset, u32 value_to_write);

static int magma_iwlwifi_spawn_hw_base(struct pci_dev *magma_iwlwifi_pci, void __iomem *hw_base){
    /* control if the pci_dev struct is safe to use as argoument for other functions */
    void __iomem * const *page_tables = pcim_iomap_table(magma_iwlwifi_pci);
    if( page_tables == NULL ){
        return -1; //TODO
    }
    hw_base = page_tables[0];
    return 0;
}

/* note that iwlwifi has only the PCIe interface, so readl/writel "bare" functions will work without any problem */

static void magma_iwlwifi_pcie_write8(void __iomem *hw_base, u32 offset, u8 value_to_write){
    /* write 1 byte (8 bits) */
    writeb(value_to_write, (hw_base + offset));
}

static void magma_iwlwifi_pcie_write32(void __iomem *hw_base, u32 offset, u8 value_to_write){
    /* write 32 bits, 'long' */
    writel(value_to_write, (hw_base + offset));
}

static u32 magma_iwlwifi_pcie_read32(void __iomem *hw_base, u32 offset){
    /* read 32 bits */
    return readl(hw_base + offset);
}

static void magma_iwlwifi_pcie_set_bitmask(void __iomem *hw_base, u32 register_to_set, u32 mask, u32 value_to_write){
    u32 temp_val;
    temp_val = magma_iwlwifi_pcie_read32(hw_base, register_to_set);
    temp_val &= ~mask;
    temp_val |= value_to_write;
    magma_iwlwifi_pcie_write32(hw_base, register_to_set, temp_val);
}

/* functions for reseting the internal iwlwifi card software state, interesting under the debugging point of view */
static void magma_iwlwifi_pcie_software_reset(void __iomem *hw_base){
    #define MAGMA_IWLWIFI_RESET_CONTROL_STATUS_REGISTER 0x020
    #define MAGMA_IWLWIFI_RESET_FLAG 0X00000080
    magma_iwlwifi_pcie_set_bitmask(hw_base, MAGMA_IWLWIFI_RESET_CONTROL_STATUS_REGISTER, MAGMA_IWLWIFI_RESET_FLAG, MAGMA_IWLWIFI_RESET_FLAG);

}

/* read 32 bit configs from the iwlwifi card */
static int magma_iwlwifi_pcie_read_config32(struct pci_dev *magma_pci_dev, u32 offset, u32 *value){
    if( sizeof(magma_pci_dev) == 0 || magma_pci_dev == NULL ){
        #define PCIE_DEV_NOT_FILLED 0x2
        return -PCIE_DEV_NOT_FILLED;
    }
    return pci_read_config_dword(magma_pci_dev, offset, value);
}

/* magma_iwlwifi_pcie_write_bytes: write 8 bits or 32 bits at the specified offset in the WLAN card */
static void magma_iwlwifi_pcie_write_bytes(void *hw_base, enum magma_iwlwifi_read_len magma_iwlwifi_readlength, u32 offset, u32 value_to_write){
    switch(magma_iwlwifi_readlength){
        case MAGMA_IWLWIFI_WRITE_BYTE:
            magma_iwlwifi_pcie_write8(hw_base, offset, value_to_write );
            break;
        case MAGMA_IWLWIFI_WRITE_LONG:
            magma_iwlwifi_pcie_write32(hw_base, offset, value_to_write );
            break;
        case MAGMA_IWLWIFI_READ_LONG:
            magma_iwlwifi_pcie_read32(hw_base, offset);
            break;
        case MAGMA_IWLWIFI_READ_CONFIG_LONG:
            /* still to think how to spawn a pci_dev struct */
            // magma_iwlwifi_pcie_read_config32();
            break;
    }
}

#endif
