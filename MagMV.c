/*
    Made by Edoardo Mantovani, 2021
        MagMa-V is a prototype for an universal 802.11 driver for Linux/Android Operating systems
            - Currently supports both iwlwifi and bcm43XX chipset family, now the work must be done for the qcacld-3.0 driver (used mostly in smartphones)

        Note that I am using the C Linux Kernel programming style, so:
            - In the top of each function body, I'll declare every variable, even if I use them at last
            - Tend to use a lot of MACRO functions, some will spawn specified structs which will be allocated safely
            - I declare the variable used in the for loop as counter before the for loop itself
            - I use only static declarations, those are suggested for the kernel modules
            - The entry point function has __init, the exit point has __exit

        Interesting readings for understanding better this work:
            - https://lupyuen.github.io/articles/wifi
            - https://github.com/erikarn/iwm/tree/master/driver (FreeBSD implementation of the iwlwifi Linux's driver)
            - https://www.kernel.org/doc/html/v5.0/driver-api/80211/index.html (Linux Kernel official page about the cfg80211 && mac80211 WiFi subsystems)

            - https://www.cattius.com/ExceptionsKernelDriver/ (Used for catching exceptions in a Kernel Module)
            - https://www.kernel.org/doc/html/latest/filesystems/debugfs.html (Used for debugging the module in the fs)
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

/* Headers for the sdio_device_id struct */
#include <linux/mod_devicetable.h>

/* Additional Headers for managing skb_buff struct and other 802.11 related data types */
#include <linux/netdevice.h>
#include <linux/nl80211.h>
#include <linux/rtnetlink.h>
#include <linux/wireless.h>
#include <linux/ieee80211.h>
#include <linux/skbuff.h>

/* Header for the debugging functions */
#include <linux/kdebug.h>

/* Header for the ssb_read / ssb_write function */
#include <linux/ssb/ssb.h>

/* Headers for managing the Linux's queue system */
#include <linux/semaphore.h>
#include <linux/workqueue.h>
#include <linux/sysfs.h>

/* Principal headers for interacting with the main Kernel, works for both Android && Linux, note that regulatory.h is used for custom regdomains */
#include <net/regulatory.h>
#include <net/cfg80211.h>
#include <net/mac80211.h>

/* Included for using the function __pm_runtime_resume */
#include <linux/pm_runtime.h>

/* Linux firmware API, used for loading the firmware of the associated device in memory */
#include <linux/firmware.h>

#include "wlan_modules/magma_broadcom.h"

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

/*
    After the discover of the 802.11 driver, the MAGMA_V_INITIALIZE_SOFTMAC() and the MAGMA_V_INITIALIZE_HARDMAC functions are used for setting up the
    specific struct which will be used to fill the cfg/ieee 80211_ops struct, note that, the various specific functions, are included from another header file.
    NOTE that in both the 2 '80211_ops' struct, the function pointer for the monitor mode isn't present, this because the network adapter can switch in monitor
    mode if and only if the user requests a new interface 'add_intf'.

*/

/* the MAGMA_V_INITIALIZE_SOFTMAC is used when the device supports effectively softMAC, this permits to initialize the ieee80211_ops struct */
#define MAGMA_V_INITIALIZE_SOFTMAC() {                                                                                              \
int magma_softmac_start_device(struct ieee80211_hw *hw);                                                                            \
void magma_softmac_stop_device(struct ieee80211_hw *hw);                                                                            \
void magma_softmac_frame_tx(struct ieee80211_hw *hw, struct ieee80211_tx_control *control, struct sk_buff *skb);                    \
int magma_softmac_configure(struct ieee80211_hw *hw, u32 changed);                                                                  \
int magma_softmac_add_interface(struct ieee80211_hw *hw, struct ieee80211_vif *vif);                                                \
int magma_softmac_change_interface(struct ieee80211_hw *hw, struct ieee80211_vif *vif, enum nl80211_iftype new_type, bool p2p);     \
int magma_softmac_start_ap_mode(struct ieee80211_hw *hw, struct ieee80211_vif *vif);                                                \
void magma_softmac_stop_ap_mode(struct ieee80211_hw *hw, struct ieee80211_vif *vif);                                                \
void magma_softmac_remove_interface(struct ieee80211_hw *hw, struct ieee80211_vif *vif);                                            \
int magma_softmac_scan(struct ieee80211_hw *hw, struct ieee80211_vif *vif, struct ieee80211_scan_request *req);                     \
struct ieee80211_ops magma_V_softmac = {                        \
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
    };                                                          \
    magma_ieee80211_ops = &magma_V_softmac;                     \
}

/* the MAGMA_V_INITIALIZE_HARDMAC is used when the device supports effectively hardMAC, this permits to initialize the cfg80211_ops struct */
#define MAGMA_V_INITIALIZE_HARDMAC() {    \
struct wireless_dev *magma_add_interface(struct wiphy *wiphy, const char *name, unsigned char name_assign_type, enum nl80211_iftype type, struct vif_params *params ); \
int magma_del_interface(struct wiphy *wiphy, struct wireless_dev *wdev); \
int magma_start_ap_mode(struct wiphy *wiphy, struct net_device *dev, struct cfg80211_ap_settings *settings); \
int magma_stop_ap_mode(struct wiphy *wiphy, struct net_device *dev);        \
int magma_scan(struct wiphy *wiphy, struct cfg80211_scan_request *request); \
struct cfg80211_ops magma_V_hardmac = {                 \
        .add_virtual_intf = magma_add_interface,        \
        .del_virtual_intf = magma_del_interface,        \
        .start_ap = magma_start_ap_mode,                \
        .stop_ap = magma_stop_ap_mode,                  \
        .scan = magma_scan,                             \
    };                                                  \
    magma_cfg80211_ops = &magma_V_hardmac;              \
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
/* Note that from here, the _GENERICX constants will be used for testing purpose only, they aren't specified in any 802.11 standards */
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
    {
            .bitrate = 150,
            .hw_value = 0x16,
    },
    {
            .bitrate = 300,
            .hw_value = 0x20,
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
    /* disable radar detection, used only in 5Ghz */
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

/* define the various error codes, not such usefull, but always good to have */
enum MagMa_V_errorcodes{
    NO_ERROR = 0,
    ERROR_NOIEEE80211 = 11,
    NO_PCI,
    NO_SDIO,
    NO_USB,
    NO_BAR_AVAILABLE = 50,
    NO_ENABLE_MEMDEV = 110,
    NO_ENABLE_REGION_REQ,
    NO_ENABLE_PCI_DEV,
    ERROR_CHRDEV_ALLOC = 200,
    ERROR_DRIVER_UNSUPPORTED = 250,
    ERROR_MASTER_PCI = 300,
}MagMa_V_errorcodes;

enum MagMa_V_returncodes{
    HAS_PCI_INTEL = 3, // WIP
    HAS_PCI_BROADCOM,   // WIP
    HAS_SDIO_BROADCOM,  // WIP
    HAS_USB_BROADCOM,   // TO DO
    HAS_USB_QUALCOMM, // TO DO
    HAS_PCI_QUALCOMM, // TO DO
}MagMa_V_returncodes;

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
    /* functions for writing/reading 32 bits from the bus */
    void (*sdio_write32)(struct ssb_device *sdio_dev, u16 offset, u32 value);
    u32 (*sdio_read32)(struct ssb_device *sdio_dev, u16 offset);
    void (*iwl_pci_read32)(void __iomem *base_addr, u32 offset, u32 value);
    void (*iwl_pci_write32)(void __iomem *base_addr, u32 offset, u32 value);
};

/* before prototyping any function, I've decided to insert the ieee80211_hw, class and device structures here */
static struct ieee80211_hw *magma_V_hardware;
static struct wiphy *magma_wireless_physical_layer;
static struct class *magma_class = NULL;
static struct device *magma_device = NULL;
static struct ieee80211_ops *magma_ieee80211_ops = NULL;
static struct cfg80211_ops *magma_cfg80211_ops = NULL;
static struct file_operations *magma_V_fops[3];

static struct magma_wlan_device_detail *magma_wlan_dev_det = NULL;

/* functions prototypes, both PCI, SDIO and USB functions are declared */
static struct magma_wlan_device_detail *magma_pci_probe(struct pci_dev *pci_dev, struct pci_device_id *pci_table_entity);
static int magma_sdio_host_claimer(struct mmc_host *host, struct mmc_ctx *ctx, atomic_t *abort);
static int softmac_detection(struct magma_wlan_device_detail *magma_dev);

/* still to be completed */
static int magma_initialize_wlan_pci(struct pci_dev *pci_wlan, enum MagMa_V_returncodes magma_switchcodes){
    int barr = 0;
    barr = pci_select_bars(pci_wlan, IORESOURCE_MEM);
    if( barr < 0 ){
        return -NO_BAR_AVAILABLE;
    }
    if( pci_enable_device_mem(pci_wlan) ){
        kfree(pci_wlan);
        return -NO_ENABLE_MEMDEV;
    }
    if( pci_request_region(pci_wlan, barr, "MagMa-V") ){
        pci_disable_device(pci_wlan);
        return -NO_ENABLE_REGION_REQ;
    }
    if( pci_enable_device(pci_wlan) ){
        return -NO_ENABLE_PCI_DEV;
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
        case HAS_PCI_QUALCOMM:
        break;

    }
    return 0;
}


#ifdef IWLWIFI_PROBE
        #include "wlan_modules/magma_intel.h"

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
    }else{
        magma_dev_detail->magma_retcodes = NO_ERROR;
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
            // TODO: return
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
    }
    /* if the code not return anything before this return, return a NO_PCI error code */
    return magma_dev_detail;
}

/* make the magma_pci_probe symbol exportable, this means that will be possible to use this function as an 'extern' for additional kernel module objects */
EXPORT_SYMBOL(magma_pci_probe);

/* safely remove driver control over PCI device */
static void magma_pci_remove(struct pci_dev *pci_device){
    pci_free_irq_vectors(pci_device);    
    pci_release_regions(pci_device);
}

/* main function used for sending Host CoMmanDs for Intel based devices using PCI bus */
static int magma_iwlwifi_send_pci_hcmd(enum magma_iwlwifi_hcmd msg){
    /* here we must be able to identify which type of device is and if is MVM or DVM */

    return 0;
}
EXPORT_SYMBOL(magma_iwlwifi_send_pci_hcmd);

#endif

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

/* write the adapter device in the magma private log shell */
static void magma_softmac_notify_attached_80211_adapter(char *msg_log_buffer, enum MagMa_V_returncodes magma_retcode){
    switch(magma_retcode){
        case HAS_PCI_INTEL:
                snprintf(msg_log_buffer, MAGM_MAX_BUFFER_SIZE, "[*]new 802.11 adapter found: Intel over PCI\n");
                break;
        case HAS_PCI_BROADCOM:
                snprintf(msg_log_buffer, MAGM_MAX_BUFFER_SIZE, "[*]new 802.11 adapter found: Broadcom over PCI\n");
                break;
        case HAS_SDIO_BROADCOM:
                snprintf(msg_log_buffer, MAGM_MAX_BUFFER_SIZE, "[*]new 802.11 adapter found: Broadcom over SDIO\n");
                break;
        case HAS_USB_BROADCOM:
                snprintf(msg_log_buffer, MAGM_MAX_BUFFER_SIZE, "[*]new 802.11 adapter found: Broadcom over USB\n");
                break;
        case HAS_USB_QUALCOMM:
                snprintf(msg_log_buffer, MAGM_MAX_BUFFER_SIZE, "[*]new 802.11 adapter found: Qualcomm over USB\n");
                break;
        case HAS_PCI_QUALCOMM:
                snprintf(msg_log_buffer, MAGM_MAX_BUFFER_SIZE, "[*]new 802.11 adapter found: Qualcomm over PCI\n");
                break;
    }
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

/* setup the emergency/log shell, this small char buffer will contain informations about the various private activities of the driver, which won't be displayed in the dmesg log (essentially I do not use printk) */
static void magma_softmac_setup_emergency_shell(struct ieee80211_hw *hw){
    #ifdef CONFIG_MAC80211_LEDS
        char *emergency_buffer;
        magma_softmac_create_led_trigger(hw, , 100, 1, emergency_buffer);
        magma_softmac_display_hardware_info(hw, emergency_buffer);
    #endif
}


/* call this function if and only if a Broadcom wlan device has been found in the SDIO bus, note that set up the 2 sdio_r/w functions, but set to 'NULL' the ones related to the iwlwifi */
#define MAGMA_SPAWN_BROADCOM_SDIO_RW() {    \
u32 magma_broadcomm_sdio_read32(struct ssb_device *sdio_dev, u16 offset){  \
	return ssb_read32(sdio_dev, offset);                        \
}                                                               \
void magma_broadcomm_sdio_write32(struct ssb_device *sdio_dev, u16 offset, u32 value){ \
	ssb_write32(sdio_dev, offset, value);                                   \
}                                                                           \
magma_wlan_dev_det->iwl_pci_read32 = NULL;                                               \
magma_wlan_dev_det->iwl_pci_write32 = NULL;                                              \
magma_wlan_dev_det->sdio_write32 = magma_broadcomm_sdio_write32;                         \
magma_wlan_dev_det->sdio_read32 = magma_broadcomm_sdio_read32;                           \
}

#define MAGMA_SPAWN_QUALCOMM_PCI_RW() { \
}

/* entry point function */
static int __init detect_available_wl0_intf(void){

    #ifndef MAC_ADDRESS_LEN
        #define MAC_ADDRESS_LEN 6
    #endif
    int i = 0;
    dev_t magma_device_t;
    /* the MAC address we are going to give to our 802.11 adapter, note that is ED:0B:AD:ED:0B:AD */
    u8 permanent_address[MAC_ADDRESS_LEN] = {0xED, 0x0B, 0xAD, 0xED, 0x0B, 0xAD};
    /* bitmask of frames that can be received by the driver OTA */
    u16 antenna_rx[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x6, 0x7, 0x8, 0x9, 0x10, 0xFF, 0xcc, 0xBAD};
    /* bitmask of frames that can be sent by the driver OTA */
    u16 antenna_tx[] = {0x00, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x99};

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
        return ERROR_DRIVER_UNSUPPORTED;
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
        /* allocate the cfg80211_ops safely */
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
            case HAS_PCI_QUALCOMM:  /* note that this is referred to the 'qtnfmac' driver */
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


