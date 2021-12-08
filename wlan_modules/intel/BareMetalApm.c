/*  © 2021 Edoardo Mantovani All Rights Reserved */

#include "magma_intel.h"
#include <linux/delay.h>

/* 
    magma_iwlwifi_manipulate_bit: this macro contains both the 'iwl_set_bit' and the 'iwl_clear_bit' functions, the choose of  'choose' determine which function will be invoked
        - choose == 0 -> iwl_set_bit
        - choose != 0 -> iwl_clear_bit
*/

#define magma_iwlwifi_manipulate_bit(choose, hw_base, register_to_set, mask) {  \
    if( choose == 0 ){                                                                          \
        magma_iwlwifi_pcie_set_bitmask(hw_base, register_to_set, mask, mask);                   \
    }else{                                                                                      \
        magma_iwlwifi_pcie_set_bitmask(hw_base, register_to_set, mask, 0);                      \
    }                                                                                           \
}


static void magma_iwlwifi_get_network_interface_card_basic_functions(){
            #define CSR_DBG_LINK_PWR_MGMT_REG           0x250
            #define CSR_RESET_LINK_PWR_MGMT_DISABLED    0x80000000
            #define CSR_HW_IF_CONFIG_REG                0x00000000
            #define CSR_HW_IF_CONFIG_REG_PREPARE        0x08000000
            #define CSR_HW_IF_CONFIG_REG_ENABLE_PME     0x10000000
			magma_iwlwifi_manipulate_bit(0, trans, CSR_DBG_LINK_PWR_MGMT_REG, CSR_RESET_LINK_PWR_MGMT_DISABLED);
			magma_iwlwifi_manipulate_bit(0, trans, CSR_HW_IF_CONFIG_REG, ( CSR_HW_IF_CONFIG_REG_PREPARE | CSR_HW_IF_CONFIG_REG_ENABLE_PME ) );
			mdelay(1);
			magma_iwlwifi_manipulate_bit(1, trans, CSR_DBG_LINK_PWR_MGMT_REG, CSR_RESET_LINK_PWR_MGMT_DISABLED);
		    mdelay(5);

	//clear_bit(STATUS_DEVICE_ENABLED, &trans->status);
	        /* Stop device's DMA activity */
	        iwl_pcie_apm_stop_master(trans);

            magma_iwlwifi_pcie_software_reset(trans);
            /* iwl_trans_pcie_sw_reset essentially reset the controller */
	/*
	 * Clear "initialization complete" bit to move adapter from
	 * D0A* (powered-up Active) --> D0U* (Uninitialized) state.
	 */
	        magma_iwlwifi_manipulate_bit(1, trans, 0x024, 0x00000004);
            usleep(5000);

            /* now  'iwl_pcie_apm_init' must be implemented */
}

