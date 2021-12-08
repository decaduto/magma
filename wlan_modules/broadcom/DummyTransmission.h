/*  © 2021 Edoardo Mantovani All Rights Reserved */

/* for 'udelay' function */
#include <asm/delay.h>
#include "magma_broadcom.h"

// STILL TO FINISH !!!

/* the DummyTransmission feature is descibed at this link: https://bcm-v4.sipsolutions.net/802.11/DummyTransmission/ and is implemented in the b43 Kernel module here: https://elixir.bootlin.com/linux/latest/source/drivers/net/wireless/broadcom/b43/main.c#L739 
    THe main purpose of this header, is to offer a simplified function which permits to emulate a physical layer frame transmission without sending anything
    OTA 
*/


/* The packet that is sent depends on the PHY type. The first 6 bytes are for the PLCP data, the remainder is an ACK packet (type 0b01, subtype 0b1101)  */


/* the procedure is illustrated here:

        Write the packet you wish to send to Template Ram, offset 0
        Write 0 to Core Register 0x568
        If the Core revision is less than 11
            Write 0 to Core Register 0x7C0 
        Otherwise
            Write 0x100 to Core Register 0x7C0 
        If you are writing an OFDM packet
            Write 0x41 to Core Register 0x50C 
        Otherwise
            Write 0x40 to Core Register 0x50C 
        If this is an N PHY, an LP PHY, or an SSLPN PHY (type 6)
            Write 0x1A02 to Core Register 0x514 
        Write 0 to Core Register 0x508
        Write 0 to Core Register 0x50A
        write 0 to Core Register 0x54C
        Write 0x14 to Core Register 0x56A
        Write 0x826 to Core Register 0x568
        Write 0 to Core Register 0x500
        If argument pa_on is false and this is an N PHY
            Read PHY Register 0x91 and save the result
            Read PHY Register 0x92 and save the result
            If the radio chanspec bitwise ANDed with 0xF000 equals 0x1000 (5 GHz)
                Set an override value to 0x180 
            Otherwise (2.4 GHz)
                Set an override value of 0x120 
            Write the override value to Phy Register 0x91
            Write the override value to Phy Register 0x92 
        If this is a N PHY (type == 4) or an SSLPN PHY (type == 6)
            Set a mask to 0xD0 
        Else if this is an LP PHY (type == 5)
            Set a mask to 0x50 
        Otherwise
            Set a mask to 0x30 
        Write the mask to Core Register 0x502
        Spinwait until Core Register 0x50E has bit 0x80 set, at most 300 usecs for OFDM packets, 2500 usecs for CCCK packets, with a delay of 10 usecs
        Spinwait until Core Register 0x50E has bit 0x400 set, at most 100 usecs, with a delay of 10 usecs
        Spinwait until Core Register 0x690 has bit 0x100 unset, at most 100 usecs, with a delay of 10 usecs
        If the input argument pa_on is zero and this is an N PHY
            Write the saved value to PHY Register 0x91
            Write the saved value to PHY Register 0x92 

*/

struct magma_fake_transmission_details{
    int core_review;
    int phy_type;
    int radio_ver;
};

struct magma_fake_transmission_ops{
    int is_ofdm : 1;
    int is_pa_on : 1;
    struct magma_fake_transmission_details *details;
};

#define magma_execute_dummy_test(idea){ \
    if( idea == 0 ){    \
        struct magma_fake_transmission_ops magma_fk = { \
            is_ofdm = 1,    \
            is_pa_on = 0,   \
        };  \
        magma_istantiate_fake_transmission(&magma_fk);  \
    }else if( idea == 1 ){  \
        struct magma_fake_transmission_ops magma_fk = { \
            is_ofdm = 0,    \
            is_pa_on = 1,   \
        };  \
        magma_istantiate_fake_transmission(&magma_fk);  \
    }else{   \
        struct magma_fake_transmission_ops magma_fk = { \
            is_ofdm = 1,    \
            is_pa_on = 1,   \
        };  \
        magma_fk = {    \
            .details = {    \
                .radio_ver = 0x2050,  \
                .phy_type = 0x08,   \
                .core_review = 12, \
            },  \
        };  \
        magma_istantiate_fake_transmission(&magma_fk);  \
    }   \
}

void magma_istantiate_fake_transmission(struct magma_fake_transmission_ops *fake_trans_ops){ /* sobstitute a struct which contains both ofdm and pa_on bool but use bit field instead */
    #define BCMA_ADDR_BASE 0x18000000	
    unsigned int i, max_loop;
	u16 value;
	u32 buffer[5] = {
		0x00000000,
		0x00D40000,
		0x00000000,
		0x01000000,
		0x00000000,
	};

	if (fake_trans_ops->is_ofdm){
		max_loop = 0x1E;
		buffer[0] = 0x000201CC;
	} else {
		max_loop = 0xFA;
		buffer[0] = 0x000B846E;
	}

	for (i = 0; i < 5; i++){
        magma_broadcom_main_io(BCMA_ADDR_BASE, MAGMA_BROADCOM_DO_RAM_WRITE32, (i * 4), buffer[i]);
    }
    #ifndef B43_MMIO_XMTSEL
        #define B43_MMIO_XMTSEL 0x568
    #endif
	magma_broadcom_main_io(BCMA_ADDR_BASE, MAGMA_BROADCOM_DO_WRITE16, B43_MMIO_XMTSEL, 0x0000);

    #define B43_MMIO_WEPCTL 0x7C0
	if (fake_trans_ops->details->core_review < 11){
		magma_broadcom_main_io(BCMA_ADDR_BASE, MAGMA_BROADCOM_DO_WRITE16, B43_MMIO_WEPCTL, 0x0000);
	}else{
		magma_broadcom_main_io(BCMA_ADDR_BASE, MAGMA_BROADCOM_DO_WRITE16, B43_MMIO_WEPCTL, 0x0100);
    }
    if( fake_trans_ops->is_ofdm ){
        value = 0x41;
    }else{
        value = 0x40;
    }
    #define B43_MMIO_TXE0_PHYCTL    0x50C
	magma_broadcom_main_io(BCMA_ADDR_BASE, MAGMA_BROADCOM_DO_WRITE16, B43_MMIO_TXE0_PHYCTL, value);
    #define B43_MMIO_TXE0_WM_0  0x508
    #define B43_MMIO_TXE0_WM_1  0x50A
    #define B43_MMIO_XMTTPLATETXPTR 0x54C
    #define B43_MMIO_XMTTXCNT   0x56A
    #define B43_MMIO_TXE0_CTL   0x500
	if( fake_trans_ops->details->phy_type == 0x04 || fake_trans_ops->details->phy_type == 0x05 || fake_trans_ops->details->phy_type == 0x08){ /* where 0x04, 0x05 and 0x08 are respectively for 802.11 N LP and LCN*/
        #ifndef B43_MMIO_TXE0_PHYCTL1
            #define B43_MMIO_TXE0_PHYCTL1   0x514
        #endif
		magma_broadcom_main_io(BCMA_ADDR_BASE, MAGMA_BROADCOM_DO_WRITE16, B43_MMIO_TXE0_PHYCTL1, 0x1A02);
    }
	magma_broadcom_main_io(BCMA_ADDR_BASE, MAGMA_BROADCOM_DO_WRITE16, B43_MMIO_TXE0_WM_0, 0x0000);
	magma_broadcom_main_io(BCMA_ADDR_BASE, MAGMA_BROADCOM_DO_WRITE16, B43_MMIO_TXE0_WM_1, 0x0000);

	magma_broadcom_main_io(BCMA_ADDR_BASE, MAGMA_BROADCOM_DO_WRITE16, B43_MMIO_XMTTPLATETXPTR, 0x0000);
	magma_broadcom_main_io(BCMA_ADDR_BASE, MAGMA_BROADCOM_DO_WRITE16, B43_MMIO_XMTTXCNT, 0x0014);
	magma_broadcom_main_io(BCMA_ADDR_BASE, MAGMA_BROADCOM_DO_WRITE16, B43_MMIO_XMTSEL, 0x0826);
	magma_broadcom_main_io(BCMA_ADDR_BASE, MAGMA_BROADCOM_DO_WRITE16, B43_MMIO_TXE0_CTL, 0x0000);
    #define B43_MMIO_TXE0_STATUS    0x50E
    #define B43_MMIO_TXE0_AUX   0x502
	switch(fake_trans_ops->details->phy_type) {
	case 0x04:
	case 0x08:
		magma_broadcom_main_io(BCMA_ADDR_BASE, MAGMA_BROADCOM_DO_WRITE16, B43_MMIO_TXE0_AUX, 0x00D0);
		break;
	case 0x05:
		magma_broadcom_main_io(BCMA_ADDR_BASE, MAGMA_BROADCOM_DO_WRITE16, B43_MMIO_TXE0_AUX, 0x0050);
		break;
	default:
		magma_broadcom_main_io(BCMA_ADDR_BASE, MAGMA_BROADCOM_DO_WRITE16, B43_MMIO_TXE0_AUX, 0x0030);
	}
	magma_broadcom_main_io(BCMA_ADDR_BASE, MAGMA_BROADCOM_DO_READ16, B43_MMIO_TXE0_AUX, 0);

	if (fake_trans_ops->details->radio_ver == 0x2050 && fake_trans_ops->details->radio_ver <= 0x5){
		magma_broadcom_main_io(BCMA_ADDR_BASE, MAGMA_BROADCOM_DO_WRITE16, 0x0051, 0x0017);
    }
	for (i = 0x00; i < max_loop; i++) {
		value = magma_broadcom_main_io(BCMA_ADDR_BASE, MAGMA_BROADCOM_DO_READ16, B43_MMIO_TXE0_STATUS, 0);
		if (value & 0x0080){
			break;
        }
		udelay(10);
	}
	for (i = 0x00; i < 0x0A; i++) {
		value = magma_broadcom_main_io(BCMA_ADDR_BASE, MAGMA_BROADCOM_DO_READ16, B43_MMIO_TXE0_STATUS, 0);
		if(value & 0x0400){
			break;
        }
		udelay(10);
	}
	for (i = 0x00; i < 0x19; i++) {
        #ifndef B43_MMIO_IFSSTAT
            #define B43_MMIO_IFSSTAT		0x690
        #endif
		value = magma_broadcom_main_io(BCMA_ADDR_BASE, MAGMA_BROADCOM_DO_READ16, B43_MMIO_IFSSTAT, 0);
		if(! (value & 0x0100) ){
			break;
        }
		udelay(10);
	}
}
EXPORT_SYMBOL(magma_istantiate_fake_transmission);

