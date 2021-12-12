#include "magma_broadcom.h"
#include <linux/netdevice.h>


int magma_broadcom_open_monitor(struct net_device *ndev){
    u32 monitor;
    magma_broadcom_send_sdio_hcmd(BRCMF_C_GET_MONITOR);

}

static netdev_tx_t magma_broadcom_start_transmit(struct sk_buff *skb, struct net_device *ndev){
    dev_kfree_skb_any(skb);
    return NETDEV_TX_OK;
}

static const struct net_device_ops magma_broadcom_mon_interface = {
    .ndo_open = magma_broadcom_open_monitor,
    .ndo_stop = magma_broadcom_stop_monitor,
    .ndo_start_xmit = magma_broadcom_start_transmit,
};


