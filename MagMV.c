/*
    Made by Edoardo Mantovani, 2021
        MagMa-V is a prototype for an universal 802.11 driver for Linux/Android Operating systems
            - Currently supports both iwlwifi and bcm43XX chipset family, now the work must be done for the qcacld-3.0 driver (used mostly in smartphones)
*/

/* Base inclusion for Linux kernel Modules */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/delay.h>

/* Headers for interacting with the PCI/PCIe Bus */
#include <linux/pci.h>

/* Headers for interacting with the SDIO Bus, may be an over-inclusion... */
#include <linux/mmc/core.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sd.h>
#include <linux/mmc/slot-gpio.h>

/* Additional Headers for managing skb_buff struct and other 802.11 related data types */
#include <linux/netdevice.h>
#include <linux/nl80211.h>
#include <linux/rtnetlink.h>
#include <linux/wireless.h>
#include <linux/ieee80211.h>
#include <linux/skbuff.h>

/* Headers for managing the Linux's queue system */
#include <linux/semaphore.h>
#include <linux/workqueue.h>
#include <linux/sysfs.h>

/* Principal headers for interacting with the main Kernel, works for both Android && Linux, note that regulatory.h is used for custom regdomains */
#include <net/regulatory.h>
#include <net/cfg80211.h>
#include <net/mac80211.h>

/* Linux firmware API, used for loading the firmware of the associated device in memory */
#include <linux/firmware.h>

#define MODULE_DESC "MagMa-V is an universal wlan driver designed for Linux/Android OSes, supports both HardMac and SoftMac 802.11 cards, full support for Intel and Broadcomm products"
#define MODULE_FW_V "MagMa-V 1.12"
/*  
    Note that cfg80211 is used for HardMac wifi cards, mainly found in smartphones, while mac80211 is used for SoftMac wifi cards, the main difference is 
    how the software manage the MLME, mac80211 (and softmac model, in general) offers a more advanced control over MLME, cause the management frames are
    built by the kernel module software, while the cfg80211 is a blackbox solution, because the PHY part (layer 1) and the MAC layer (layer 3) are managed by the
    WiFi card firmware.    
*/
MODULE_AUTHOR("Edoardo Mantovani");
MODULE_VERSION("666");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(MODULE_DESC);
MODULE_INFO(intree, "Y");

/* the MAGMA_V_INITIALIZE_SOFTMAC is used when the device supports effectively softMAC, this permits to initialize the ieee80211_ops struct */
#define MAGMA_V_INITIALIZE_SOFTMAC() {    \
int magma_softmac_start_device(struct ieee80211_hw *hw); \
void magma_softmac_stop_device(struct ieee80211_hw *hw); \
void magma_softmac_frame_tx(struct ieee80211_hw *hw, struct ieee80211_tx_control *control, struct sk_buff *skb);    \
int magma_softmac_configure(struct ieee80211_hw *hw, u32 changed);   \
int magma_softmac_add_interface(struct ieee80211_hw *hw, struct ieee80211_vif *vif);  \
int magma_softmac_change_interface(struct ieee80211_hw *hw, struct ieee80211_vif *vif, enum nl80211_iftype new_type, bool p2p);    \
int magma_softmac_start_ap_mode(struct ieee80211_hw *hw, struct ieee80211_vif *vif);   \
void magma_softmac_stop_ap_mode(struct ieee80211_hw *hw, struct ieee80211_vif *vif);   \
void magma_softmac_remove_interface(struct ieee80211_hw *hw, struct ieee80211_vif *vif);  \
int magma_softmac_scan(struct ieee80211_hw *hw, struct ieee80211_vif *vif, struct ieee80211_scan_request *req); \
struct ieee80211_ops magma_V_softmac = {   \
        .tx = magma_softmac_frame_tx,                           \
        .start = magma_softmac_start_device,                    \
        .add_interface = magma_softmac_add_interface,           \
        .remove_interface = magma_softmac_remove_interface,     \
        .stop = magma_softmac_stop_device,                      \
        .config = magma_softmac_configure,                      \
        .change_interface = magma_softmac_change_interface,     \
        .start_ap = magma_softmac_start_ap_mode,                \
        .stop_ap = magma_softmac_stop_ap_mode,                  \
        .hw_scan = magma_softmac_scan,                          \
    };                                                  \
    magma_ieee80211_ops = &magma_V_softmac;             \
}

/* the MAGMA_V_INITIALIZE_HARDMAC is used when the device supports effectively hardMAC, this permits to initialize the cfg80211_ops struct */
#define MAGMA_V_INITIALIZE_HARDMAC() {    \
struct wireless_dev *magma_add_interface(struct wiphy *wiphy, const char *name, unsigned char name_assign_type, enum nl80211_iftype type, struct vif_params *params ); \
int magma_del_interface(struct wiphy *wiphy, struct wireless_dev *wdev); \
int magma_start_ap_mode(struct wiphy *wiphy, struct net_device *dev, struct cfg80211_ap_settings *settings); \
int magma_stop_ap_mode(struct wiphy *wiphy, struct net_device *dev); \
int magma_scan(struct wiphy *wiphy, struct cfg80211_scan_request *request); \
struct cfg80211_ops magma_V_hardmac = {   \
        .add_virtual_intf = magma_add_interface,        \
        .del_virtual_intf = magma_del_interface,        \
        .start_ap = magma_start_ap_mode,                \
        .stop_ap = magma_stop_ap_mode,                  \
        .scan = magma_scan,                             \
    };                                                  \
    magma_cfg80211_ops = &magma_V_hardmac;             \
}

/*
    How the module works:
        - Detect the various PCIe Product IDs, if matches with the index table, set the global status flag for pci to 'true'

*/




/* the CIPHER_SUITE constants are used for identifying how packets are encrypted, used in wiphy */
#define CIPHER_SUITE_USE_GROUP 		0x000FAC00
#define CIPHER_SUITE_TKIP 		    0x000FAC02
#define CIPHER_SUITE_CCMP 		    0x000FAC04
#define CIPHER_SUITE_AES_CMAC 		0x000FAC06
#define CIPHER_SUITE_GCMP 		    0x000FAC08
#define CIPHER_SUITE_GENERIC1 		0x000FAC10
#define CIPHER_SUITE_GENERIC2 		0x000FAC12
#define CIPHER_SUITE_GENERIC3 		0x000FAC14

/* note that the GENERICx cipher suite is still a Work In Progress, in the future everything will be explained */
static const u32 magm_V_supported_cipher_suite[] = {
	CIPHER_SUITE_TKIP,
	CIPHER_SUITE_CCMP,
	CIPHER_SUITE_AES_CMAC,
	CIPHER_SUITE_USE_GROUP,
	CIPHER_SUITE_GCMP,
	CIPHER_SUITE_GENERIC1,
	CIPHER_SUITE_GENERIC2,
	CIPHER_SUITE_GENERIC3,
};

/* define the various maximun limit that interfaces can have */
static struct ieee80211_iface_limit interfacelimit[] = {
		{
		    .max = 1,
		    .types = BIT(NL80211_IFTYPE_AP),
		},
		{
		    .max = 1,
		    .types = BIT(NL80211_IFTYPE_MONITOR),
		},
        {
            /* the '10' unspecified interfaces are just a placeholder for handling the 'new_interface' NL80211 command */ 
            .max = 10,
            .types = BIT(NL80211_IFTYPE_UNSPECIFIED),
        },
};

const struct ieee80211_iface_combination interfacecombination = {
		.max_interfaces = 10,
		.beacon_int_infra_match = true,
		.limits = interfacelimit,
		.n_limits = ARRAY_SIZE(interfacelimit),
};

/* define supported channels */
static struct ieee80211_channel supported_channels[] = {
	{
		.band = NL80211_BAND_2GHZ,
		.hw_value = 6,
		.center_freq = 2437,
	}
};

// TODO: study this
static struct ieee80211_rate available_rates[] = {
	{
	    	.bitrate = 10,
	    	.hw_value = 0x1,
	},
	{
	    	.bitrate = 20,
	    	.hw_value = 0x2,
	},
	{
            .bitrate = 55,
	    	.hw_value = 0x4,
	},
	{
	        .bitrate = 110,
	        .hw_value = 0x8,
	},
};

/* Access control list configuration struct for MAC access control */
const struct cfg80211_acl_data acldata = {
        .acl_policy = NL80211_ACL_POLICY_ACCEPT_UNLESS_LISTED,
        .n_acl_entries = 0,
};

/* define supported bands */
static struct ieee80211_supported_band available_bands = {
	.ht_cap.cap = IEEE80211_HT_CAP_SGI_20,
	.ht_cap.ht_supported = false,

	.channels = supported_channels,
	.n_channels = ARRAY_SIZE(supported_channels),

	.bitrates = available_rates,
	.n_bitrates = ARRAY_SIZE(available_rates),
};

/* define how the card is configured, note that .radar_enabled is used for the radar scan in 5Ghz networks, is set to "false" :) */
static struct ieee80211_conf card_configuration = {
    /* note that MONITOR MODE will plays a foundamental role, so every wiphy flag which show the monitor mode to the system, must be enabled */
    .flags = (BIT(IEEE80211_CONF_MONITOR) | BIT(IEEE80211_CONF_PS) | BIT(IEEE80211_CONF_IDLE)),
    .listen_interval = 1,
    .radar_enabled = false,
    /* the ps_timeout options permit to save energy (power save) when the host is in S1 */
    .dynamic_ps_timeout = 1,
};

#define MAGM_MAX_BUFFER_SIZE (1 << 8)

/* the Magm_universal_flags is the first non-linux struct used for defining the device buses */
struct Magm_universal_flags{
    unsigned int is_pci  : 1;
    unsigned int is_sdio : 1;
    unsigned int is_usb  : 1;
    unsigned int unsupported_driver : 1;
    unsigned char private_buffer[MAGM_MAX_BUFFER_SIZE];
}Magm_universal_flags;

/* initialize with "0" */
static struct Magm_universal_flags magm_main_V = {
    .is_pci  = 0,
    .is_sdio = 0,
    .is_usb  = 0,
    .unsupported_driver = 0,
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
    {0,}
};

MODULE_DEVICE_TABLE(pci, magm_supported_pci);

/* define the various error codes, not such usefull, but always good to have */
enum MagMa_V_errorcodes{
    ERROR_NOIEEE80211 = 11,
    NO_PCI,
    NO_SDIO,
    NO_BAR_AVAILABLE = 50,
    NO_ENABLE_MEMDEV = 110,
    NO_ENABLE_REGION_REQ,
    NO_ENABLE_PCI_DEV,
    ERROR_CHRDEV_ALLOC = 200,
}MagMa_V_errorcodes;

enum MagMa_V_returncodes{
    HAS_PCI_INTEL = 3,
    HAS_PCI_BROADCOM,
    HAS_SDIO_BROADCOM,
    HAS_USB_BROADCOM,
    HAS_USB_QUALCOMM,
}MagMa_V_returncodes;

/* HCMD enums */

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
	D3_CONFIG_CMD = 0xd3,
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
	struct brcmf_pub *drvr;
	enum magma_broadcom_bus_state state;
	uint maxctl;
	u32 chip;
	u32 chiprev;

	const struct brcmf_bus_ops *ops;
	struct brcmf_bus_msgbuf *msgbuf;
};

struct brcmf_sdio_dev {
	struct sdio_func *func1;
	struct sdio_func *func2;
	u32 sbwad;			/* Save backplane window address */
	struct brcmf_core *cc_core;	/* chipcommon core info struct */
	struct brcmf_sdio *bus;
	struct device *dev;
	struct brcmf_bus *bus_if;
	struct brcmf_mp_device *settings;
	bool oob_irq_requested;
	bool sd_irq_requested;
	bool irq_en;			/* irq enable flags */
	spinlock_t irq_en_lock;
	bool sg_support;
	uint max_request_size;
	ushort max_segment_count;
	uint max_segment_size;
	uint txglomsz;
	struct sg_table sgtable;
	#ifndef BRCMF_FW_NAME_LEN
	        #define	BRCMF_FW_NAME_LEN       320
	#endif
	char fw_name[BRCMF_FW_NAME_LEN];
	char nvram_name[BRCMF_FW_NAME_LEN];
	bool wowl_enabled;
	enum magma_broadcom_sdiod_state state;
	struct brcmf_sdiod_freezer *freezer;
};

struct magma_broadcomm_sdio_bus{
	union {
		struct brcmf_sdio_dev *sdio;
		struct brcmf_usbdev *usb;
		struct brcmf_pciedev *pcie;
	} bus_priv;
	enum magma_broadcom_bus_protocol_type proto_type;
	struct device *dev;
	struct brcmf_pub *drvr;
	enum magma_broadcom_bus_state state;
	uint maxctl;
	u32 chip;
	u32 chiprev;

	const struct brcmf_bus_ops *ops;
	struct brcmf_bus_msgbuf *msgbuf;
}magma_broadcomm_sdio_bus;

struct brcmf_sdio_count {
	ulong tx_ctlerrs;	/* Err of sending ctrl frames */
	ulong tx_ctlpkts;	/* Ctrl frames sent to dongle */
	ulong rx_ctlerrs;	/* Err of processing rx ctrl frames */
	ulong rx_ctlpkts;	/* Ctrl frames processed from dongle */
	ulong rx_readahead_cnt;	/* packets where header read-ahead was used */
};

struct brcmf_sdio {
	struct brcmf_sdio_dev *sdiodev;	/* sdio device handler */
	struct brcmf_chip *ci;	/* Chip info struct */
	struct brcmf_core *sdio_core; /* sdio core info struct */


	struct sk_buff *glomd;	/* Packet containing glomming descriptor */
	struct sk_buff_head glom; /* Packet list for glommed superframe */

	u8 *rxbuf;		/* Buffer for receiving control packets */
	uint rxblen;		/* Allocated length of rxbuf */
	u8 *rxctl;		/* Aligned pointer into rxbuf */
	u8 *rxctl_orig;		/* pointer for freeing rxctl */
	uint rxlen;		/* Length of valid data in buffer */
	spinlock_t rxctl_lock;	/* protection lock for ctrl frame resources */

	u8 sdpcm_ver;	/* Bus protocol reported by dongle */

	bool intr;		/* Use interrupts */
	bool poll;		/* Use polling */
	atomic_t ipend;		/* Device interrupt is pending */
	uint spurious;		/* Count of spurious interrupts */
	uint pollrate;		/* Ticks between device polls */
	uint polltick;		/* Tick counter */

#ifdef DEBUG
	uint console_interval;
	struct brcmf_console console;	/* Console output polling support */
	uint console_addr;	/* Console address from shared struct */
#endif				/* DEBUG */

	uint clkstate;		/* State of sd and backplane clock(s) */
	s32 idletime;		/* Control for activity timeout */
	s32 idlecount;		/* Activity timeout counter */
	s32 idleclock;		/* How to set bus driver when idle */
	bool rxflow_mode;	/* Rx flow control mode */
	bool rxflow;		/* Is rx flow control on */
	bool alp_only;		/* Don't use HT clock (ALP only) */

	u8 *ctrl_frame_buf;
	u16 ctrl_frame_len;
	bool ctrl_frame_stat;
	int ctrl_frame_err;

	spinlock_t txq_lock;		/* protect bus->txq */
	wait_queue_head_t ctrl_wait;
	wait_queue_head_t dcmd_resp_wait;

	struct timer_list timer;
	struct completion watchdog_wait;
	struct task_struct *watchdog_tsk;

	struct workqueue_struct *brcmf_wq;
	struct work_struct datawork;
	bool dpc_triggered;

	struct brcmf_sdio_count sdcnt;
	bool sleeping;
};

/* magma_wlan_fw_type: enum which contains all constants related to the iwlwifi / bcm Wifi blobs */
enum magma_wlan_fw_type{
    IWLWIFI_SERIES5000,
    IWLWIFI_SERIES7000,
    IWLWIFI_SERIES8000,
    IWLWIFI_SERIES9000,
    IWLWIFI_SERIES22000,
};
/* struct wlan_device_detail: struct used in magma_pci_probe, reports the class of the detected PCI device */
struct magma_wlan_device_detail{
    int is_softmac : 1;
    enum MagMa_V_returncodes magma_retcodes;
    enum magma_wlan_fw_type fw_ty;
    struct device (*convert_fw_type_to_blob)(enum magma_wlan_fw_type fw_ty);
    int (*load_fw)(struct device *wlan_device, enum magma_wlan_fw_type fw_ty, int flags);
    int (*direct_load_fw)(struct device *wlan_device, enum magma_wlan_fw_type fw_ty);
};

/* before prototyping any function, I've decided to insert the ieee80211_hw, class and device structures here */
static struct ieee80211_hw *magma_V_hardware;
static struct wiphy *magma_wireless_physical_layer;
static struct class *magma_class = NULL;
static struct device *magma_device = NULL;
static struct ieee80211_ops *magma_ieee80211_ops = NULL;
static struct cfg80211_ops *magma_cfg80211_ops = NULL;
static struct file_operations *magma_V_fops[2];

static struct magma_wlan_device_detail *magma_wlan_dev_det = NULL;

/* functions prototypes, both PCI, SDIO and USB functions are declared */
static struct magma_wlan_device_detail *magma_pci_probe(struct pci_dev *pci_dev, struct pci_device_id *pci_table_entity);
static int magma_initialize_wlan_pci(struct pci_dev *pci_wlan, enum MagMa_V_returncodes magma_switchcodes);
static void magma_pci_remove(struct pci_dev *pci_device);
static int magma_broadcom_send_sdio_hcmd(enum magma_broadcom_hcmd msg);
static int magma_iwlwifi_send_pci_hcmd(enum magma_iwlwifi_hcmd msg);
static int magma_sdio_host_claimer(struct mmc_host *host, struct mmc_ctx *ctx, atomic_t *abort);
static int softmac_detection(struct magma_wlan_device_detail *magma_dev);

/* still to be completed */
static int magma_initialize_wlan_pci(struct pci_dev *pci_wlan, enum MagMa_V_returncodes magma_switchcodes){
    int barr = 0;
    barr = pci_select_bars(pci_wlan, IORESOURCE_MEM);
    if( barr < 0 ){
        return NO_BAR_AVAILABLE;
    }
    if( pci_enable_device_mem(pci_wlan) ){
        kfree(pci_wlan);
        return NO_ENABLE_MEMDEV;
    }
    if( pci_request_region(pci_wlan, barr, "MagMa-V") ){
        pci_disable_device(pci_wlan);
        return NO_ENABLE_REGION_REQ;
    }
    if( pci_enable_device(pci_wlan) ){
        return NO_ENABLE_PCI_DEV;
    }
    pci_set_master(pci_wlan);
    
    switch(magma_switchcodes){
        case HAS_PCI_INTEL:
    
        break;

        case HAS_PCI_BROADCOM:

        break;
    
        /* those cases are not contemplated, I declared them only for toggle the compiler warnings  */
        case HAS_SDIO_BROADCOM:
        case HAS_USB_BROADCOM:
        case HAS_USB_QUALCOMM:
        break;

    }
    return 1;
}


/* PCI function for probying the device */
static struct magma_wlan_device_detail *magma_pci_probe(struct pci_dev *pci_dev, struct pci_device_id *pci_table_entity){
    int i = 0;
    /* allocate a blank return magma_wlan_device_detail struct */    
    struct magma_wlan_device_detail *magma_dev_detail = kmalloc(sizeof(magma_dev_detail), GFP_KERNEL);
    /* do not alloc the pci_dev structure, cause will be allocated automatically later */
    struct pci_dev *magma_pci_device;
    /* set to NO_PCI as default, if some good result is found, change the flag and return the struct as is */
    magma_dev_detail->magma_retcodes = NO_PCI;
    /* try to get the device, note that the vendor ID has been set to 'any' for semplyfing anything, we play only on the product ID, the third arg, NULL, indicates the kernel to do a new PCI scan for detecting the device */
    /* quit if the  pci_dev_present returns '0', meaning that no matchable vendor-product ID has been found in the host */
    if( pci_dev_present(magm_supported_pci) == 0 ){
        return magma_dev_detail;
    }
    #ifndef TOTAL_PCI_PAIR_INDEX
        #define TOTAL_PCI_PAIR_INDEX 113
    #endif
    /* 'TOTAL_PCI_PAIR_INDEX' indicates the total number of the elements inside the pci_device_id struct */
    for(; i <= TOTAL_PCI_PAIR_INDEX; i++){
        /* try to identify the pci device using the 'pci_get_device' function, this will iterate the Vendor and the Product ID until the last element of the 'pci_device_id' struct */
        magma_pci_device = pci_get_device(magm_supported_pci[i].vendor, magm_supported_pci[i].device, NULL);
        /* break the search if and only if the returned structure has a registered 'struct device' */
        if( device_is_registered(&magma_pci_device->dev) && sizeof(magma_pci_device) != 0 ){
            /* set the global flag for identifying a general 802.11 PCI device, now must determine it's vendor */
            magm_main_V.is_pci = 1;
            break;
        }
    }      
        #ifndef IWLWIFI_PCI_PAIR_INDEX
            #define IWLWIFI_PCI_PAIR_INDEX 113
        #endif
        if( i >= IWLWIFI_PCI_PAIR_INDEX ){
            if( magma_initialize_wlan_pci(magma_pci_device, HAS_PCI_INTEL) < 0 ){
                    /* still to think on that */
                }
                /* return the INTEL PCI retcode */
                magma_dev_detail->magma_retcodes = HAS_PCI_INTEL;
                return magma_dev_detail;
            }else{
                /* return the BROADCOM PCI retcode */
                if( magma_initialize_wlan_pci(magma_pci_device, HAS_PCI_BROADCOM) < 0 ){
                    // TODO: think on it return ;
                }
                magma_dev_detail->magma_retcodes = HAS_PCI_BROADCOM;
                return magma_dev_detail;
            }
    /* if the code not return anything before this return, return a NO_PCI error code */
    return magma_dev_detail;
}

/* make the magma_pci_probe symbol exportable, this means that will be possible to use this function as an 'extern' for additional kernel module objects */
EXPORT_SYMBOL(magma_pci_probe);

/* safely remove driver control over PCI device */
static void magma_pci_remove(struct pci_dev *pci_device){
    pci_free_irq_vectors(pci_device);    
    //pci_release_region(pci_device);
}

/* SDIO function for probying the device */
/*
static int magma_sdio_probe(void){



    return NO_SDIO;
}
*/

#ifndef SOFTMAC_DEF
    #define NO_SOFTMAC -1
    #define HAVE_SOFTMAC 1
#endif

static int softmac_detection(struct magma_wlan_device_detail *magma_dev){
    if( magma_wlan_dev_det->is_softmac ){
        return HAVE_SOFTMAC;
    }else{
        return NO_SOFTMAC;
    }
}

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
	} else
    wake_up(&host->wq);
	spin_unlock_irqrestore(&host->lock, flags);
	remove_wait_queue(&host->wq, &wait);
        	// if(pm)
		//pm_runtime_get_sync(mmc_dev(host));

	return stop;
}

/* main function used for sending Host CoMmanDs for Intel based devices using PCI bus */
static int magma_iwlwifi_send_pci_hcmd(enum magma_iwlwifi_hcmd msg){
    /* here we must be able to identify which type of device is and if is MVM or DVM */

    return 0;
}

/* main function used for sending Host CoMmanDs for broadcomm devices using SDIO bus */
static int magma_broadcom_send_sdio_hcmd(enum magma_broadcom_hcmd msg){
        wait_queue_head_t wireless_wait_queue;
        spinlock_t wifi_spinlock;
	    int ret;
        int msg_len;
        unsigned char *cmd;
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

/* write in the private magma driver data buffer the led names */
static void magma_softmac_display_hardware_info(struct ieee80211_hw *hw, char *msg_log_buffer){
    /* reading the source code here https://elixir.bootlin.com/linux/v5.10.75/source/include/net/mac80211.h#L4275 I pointed out that for using the LED functions, the CONFIG_MAC80211_LEDS configs must be enabled in the kernel */
    #ifdef CONFIG_MAC80211_LEDS

    /* macro function for clearing the buffer used for the information shell, wait for 2 seconds */
    #define LOG_SHELL_CLEAR() { \
        memset(msg_log_buffer, 0x0, MAGM_MAX_BUFFER_SIZE);    \
        msleep(2000);                                           \
    }

    /* do a first refresh of the buffer, just for be secure to delete all old data, if any */
        LOG_SHELL_CLEAR();
        if( ieee80211_get_tx_led_name(hw) != NULL ){
            snprintf(msg_log_buffer, MAGM_MAX_BUFFER_SIZE, "[*]softmac tx led name: %s\n", ieee80211_get_tx_led_name(hw));
        }else{
            snprintf(msg_log_buffer, MAGM_MAX_BUFFER_SIZE, "tx led name is not supported!\n");
        }
        LOG_SHELL_CLEAR();
        if( ieee80211_get_rx_led_name(hw) != NULL ){
            snprintf(msg_log_buffer, MAGM_MAX_BUFFER_SIZE, "[*]softmac rx led name: %s\n", ieee80211_get_rx_led_name(hw));
        }else{
            snprintf(msg_log_buffer, MAGM_MAX_BUFFER_SIZE, "rx led name is not supported!\n");
        }
        LOG_SHELL_CLEAR();
        if( ieee80211_get_radio_led_name(hw) != NULL ){
            snprintf(msg_log_buffer, MAGM_MAX_BUFFER_SIZE, "[*]softmac radio led name: %s", ieee80211_get_radio_led_name(hw));
        }else{
            snprintf(msg_log_buffer, MAGM_MAX_BUFFER_SIZE, "radio led name is not supported!\n");
        }
        LOG_SHELL_CLEAR();
        if( ieee80211_get_assoc_led_name(hw) != NULL ){
            snprintf(msg_log_buffer, MAGM_MAX_BUFFER_SIZE, "[*]assoc led name: %s", ieee80211_get_assoc_led_name(hw));
        }else{
            snprintf(msg_log_buffer, MAGM_MAX_BUFFER_SIZE, "assoc led name is not supported!\n");
        }
    #endif
}

static void magma_softmac_create_led_trigger(struct ieee80211_hw *hw, enum ieee80211_tpt_led_trigger_flags flags, int throughput, int blink_time, char *msg_log_buffer){
    #ifdef CONFIG_MAC80211_LEDS
    const char *custom_led_trigger;
    /* craft the ieee80211_tpt_blink struct if and only if the kernel is compiled with the 'CONFIG_MAC80211_LEDS' flag activated */
    #define CREATE_CUSTOM_LED_TRIGGER() {   \
        struct ieee80211_tpt_blink magma_tmp_blink_struct = {    \
            .throughput = throughput,                            \
            .blink_time = blink_time,                            \
        };                                                       \
    custom_led_trigger = ieee80211_create_tpt_led_trigger(hw, flags, &magma_tmp_blink_struct, sizeof(magma_tmp_blink_struct));    \
    }
    CREATE_CUSTOM_LED_TRIGGER();
    /* if the result of the 'ieee80211_create_tpt_led_trigger' is positive, print it in the log shell */
    if( custom_led_trigger != NULL ){
        snprintf(msg_log_buffer, MAGM_MAX_BUFFER_SIZE, "[*]custom led name: %s\n", custom_led_trigger);
    }else{
        snprintf(msg_log_buffer, MAGM_MAX_BUFFER_SIZE, "custom led name is not supported!\n");
    }
    #endif
}

/* entry point function */
static int __init detect_available_wl0_intf(void){

    #ifndef MAC_ADDRESS_LEN
        #define MAC_ADDRESS_LEN 6
    #endif
    int i = 0;
    dev_t magma_device_t;
    /* the MAC address we are going to give to our 802.11 adapter, note that is ED:0B:AD:ED:0B:AD */
    static u8 permanent_address[MAC_ADDRESS_LEN] = {0xED, 0x0B, 0xAD, 0xED, 0x0B, 0xAD};
    /* bitmask of frames that can be received by the driver OTA */
    static u16 antenna_rx[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x6, 0x7, 0x8, 0x9, 0x10, 0xFF, 0xcc, 0xBAD};
    /* bitmask of frames that can be sent by the driver OTA */
    static u16 antenna_tx[] = {0x00, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x99};

    #ifndef MAX_BUFFERSIZE_PVT
        #define MAX_BUFFERSIZE_PVT ( 1 << 6 )
    #endif

    /* allocate the private buffer memory pool, this will be used as internal data array for storing vendor based data, has only 2^6 bytes */
    void *privateBufferSizePool = kmalloc(GFP_KERNEL, MAX_BUFFERSIZE_PVT);
    struct ieee80211_txrx_stypes *magma_V_management_bits = (struct ieee80211_txrx_stypes*)kmalloc( ( sizeof(antenna_rx) + sizeof(antenna_tx) ), GFP_KERNEL);

    /* initialize the buffer space, this will be used as the module built in buffer for the internal console */
    memset(magm_main_V.private_buffer, 0x0, MAGM_MAX_BUFFER_SIZE);
    for(; i <= 2; i++){
        magma_V_fops[i]->owner = THIS_MODULE;
    }

    for(i = 0; i <= 1; i++){
        magma_pci_probe(NULL, NULL);
    }

    /* fetch the flag set in the Magm_universal_flags struct from the above function, exit if 'unsupported_driver' flag is set */
    if( Magm_universal_flags.unsupported_driver == 1 ){
        
    }
    /* allocate the class and device struct, for pairing with the wiphy/ieee80211_hw struct for our driver */
    if( alloc_chrdev_region(&magma_device_t, 0, 2, "MagMa-WiFi") ){
        return ERROR_CHRDEV_ALLOC;
    }
    magma_class = class_create(THIS_MODULE, "MagMa-WiFi");
    // TODO: init the device






    /* if softmac_detection() function returns that the current device is a SOFTmac one, use mac80211 structures and functions, use basic wiphy otherwise */
    if( softmac_detection(magma_wlan_dev_det) ){
        /* allocate safely the ieee80211_hw struct, defined before only as a simple pointer */
        MAGMA_V_INITIALIZE_SOFTMAC();
        magma_V_hardware = ieee80211_alloc_hw(sizeof(struct ieee80211_ops), magma_ieee80211_ops);
        if( sizeof(magma_V_hardware) == 0 ){
            // TODO
            kfree(magma_V_hardware);
            return ERROR_NOIEEE80211;
        }
        /* start the filling of the wiphy struct, insert the MAC_ADDRESS, insert various data and structures defined above */
        strncpy(magma_V_hardware->wiphy->perm_addr, permanent_address, MAC_ADDRESS_LEN);
        magma_V_hardware->wiphy->interface_modes = ( BIT(NL80211_IFTYPE_AP) | BIT(NL80211_IFTYPE_MONITOR) | BIT(NL80211_IFTYPE_UNSPECIFIED) );
        magma_V_hardware->wiphy->flags = (WIPHY_FLAG_4ADDR_AP | WIPHY_FLAG_AP_UAPSD | WIPHY_FLAG_HAVE_AP_SME | WIPHY_FLAG_HAS_CHANNEL_SWITCH | WIPHY_FLAG_AP_PROBE_RESP_OFFLOAD);
        magma_V_hardware->wiphy->bands[NL80211_BAND_2GHZ] = &available_bands;
        /* set the maximun size of Information Elements to 2285 bytes */
        magma_V_hardware->wiphy->max_scan_ie_len = 2285;
        #ifndef IEEE80211_MAX_SCAN 
                #define IEEE80211_MAX_SCAN 128
                magma_V_hardware->wiphy->max_scan_ssids = IEEE80211_MAX_SCAN;
        #endif
        #ifndef IEEE80211_MAX_REQUEST_SCAN
                #define IEEE80211_MAX_REQUEST_SCAN 128
                magma_V_hardware->wiphy->max_scan_ssids = IEEE80211_MAX_REQUEST_SCAN;
        #endif 

        magma_V_hardware->wiphy->cipher_suites = magm_V_supported_cipher_suite;
        /* NOTE that ARRAY_SIZE is like a sizeof(), fill the n_XXX_XXX only for completeness */
        magma_V_hardware->wiphy->n_cipher_suites = ARRAY_SIZE(magm_V_supported_cipher_suite);
        magma_V_hardware->wiphy->iface_combinations = &interfacecombination;
        /* n_iface_combinations is the result of 10 + 1 +1 */
        magma_V_hardware->wiphy->n_iface_combinations = 12;
        strncpy(magma_V_hardware->wiphy->fw_version, MODULE_FW_V, sizeof(magma_V_hardware->wiphy->fw_version));
        magma_V_hardware->wiphy->hw_version = 0x3f8ccccd;
        magma_V_hardware->wiphy->max_ap_assoc_sta = 0;
        magma_V_hardware->wiphy->max_num_csa_counters = 0;
        /* insert various wiphy flags, for having idea of what I've inserted, please consult https://elixir.bootlin.com/linux/latest/source/include/uapi/linux/nl80211.h#L5790 */
        magma_V_hardware->wiphy->features = (NL80211_FEATURE_INACTIVITY_TIMER | NL80211_FEATURE_CELL_BASE_REG_HINTS | NL80211_FEATURE_LOW_PRIORITY_SCAN | NL80211_FEATURE_AP_SCAN | NL80211_FEATURE_SCAN_FLUSH | NL80211_FEATURE_FULL_AP_CLIENT_STATE |
        NL80211_FEATURE_ACTIVE_MONITOR | NL80211_FEATURE_AP_MODE_CHAN_WIDTH_CHANGE | NL80211_FEATURE_WFA_TPC_IE_IN_PROBES | NL80211_FEATURE_TX_POWER_INSERTION | NL80211_FEATURE_ND_RANDOM_MAC_ADDR | NL80211_FEATURE_MAC_ON_CREATE);
        magma_V_management_bits->rx = antenna_rx;
        magma_V_management_bits->tx = antenna_tx;
        magma_V_hardware->wiphy->mgmt_stypes = magma_V_management_bits;
        /* use custom regulatory codes */
        if( true ){
            magma_V_hardware->wiphy->regulatory_flags = REGULATORY_CUSTOM_REG;
        }
        magma_V_hardware->wiphy->max_remain_on_channel_duration = 1;
        /* signal strength, increasing from 0 through 100 */
        magma_V_hardware->wiphy->signal_type = CFG80211_SIGNAL_TYPE_UNSPEC;
        magma_V_hardware->wiphy->registered = true;
        /* assign the Memory Pool allocated before */
        magma_V_hardware->priv = privateBufferSizePool;
        magma_V_hardware->conf = card_configuration;
        /* as the cfg80211 documentation explains, when rate_control_algorithm is set to NULL, means that 'minstrel' algo. will be used */
        magma_V_hardware->rate_control_algorithm = NULL;
        magma_V_hardware->flags[5] = BIT(IEEE80211_HW_HAS_RATE_CONTROL) | BIT(IEEE80211_HW_SIGNAL_DBM) | BIT(IEEE80211_HW_MFP_CAPABLE) | BIT(IEEE80211_HW_WANT_MONITOR_VIF) | BIT(IEEE80211_HW_SW_CRYPTO_CONTROL) | BIT(IEEE80211_HW_SUPPORTS_DYNAMIC_PS);
        magma_V_hardware->max_signal = 100;
        /* register the ieee80211_hw struct, if fails, free everything and return an error code */
        if( ieee80211_register_hw(magma_V_hardware) < 0 ){
                /* NOTE: must write function which analyze the specific struct bytes and says if is possible to free it or not */
                kfree(magma_V_management_bits);
                kfree(magma_V_hardware);
                return ERROR_NOIEEE80211;
        }
    /* accoppiate the ieee80211_hw struct with the device struct */
    SET_IEEE80211_DEV(magma_V_hardware, magma_device);
    /* if the card is not SOFTMAC based, must be HARDMAC based, so we will use only the cfg80211 layer, now we must play with wiphy struct, instead of the ieee80211_hw */
    }else{
        MAGMA_V_INITIALIZE_HARDMAC();

        /* if the custom regulatory flag is set */
        if( true ){
        #define APPLY_CUSTOM_REGULATORY() { \
            struct ieee80211_reg_rule magma_temp_reg_rules[] = {  \
                {                                                 \
                .flags = 0,                                       \
                },                                                \
            };                                                    \
            struct ieee80211_regdomain magma_temp_regdomain = {     \
                .dfs_region = NL80211_DFS_UNSET,                    \
                .n_reg_rules = ARRAY_SIZE(magma_temp_reg_rules),    \
            };                                                      \
            wiphy_apply_custom_regulatory(magma_wireless_physical_layer, &magma_temp_regdomain);    \
            }
            APPLY_CUSTOM_REGULATORY();
            }
        }
    return 0;
}


static void __exit deallocate_wl0_intf(void){
    if( softmac_detection(magma_wlan_dev_det) ){
        ieee80211_unregister_hw(magma_V_hardware);
        ieee80211_free_hw(magma_V_hardware);
        switch( magma_wlan_dev_det->magma_retcodes ){
            case HAS_PCI_INTEL:
            //magma_pci_remove();
            break;
            case HAS_PCI_BROADCOM:

            break;
            case HAS_SDIO_BROADCOM:
            break;
            case HAS_USB_QUALCOMM:
            break;
            case HAS_USB_BROADCOM:
            break;
        }
    }else{
        wiphy_unregister(magma_wireless_physical_layer);
        wiphy_free(magma_wireless_physical_layer);
    }
}

/* when the module is loaded, the first function which will be executed is 'detect_available_wl0_intf' */
module_init(detect_available_wl0_intf);
/* when the module is unloaded, the last function which will be executed is 'deallocate_wl0_intf' */
module_exit(deallocate_wl0_intf);


