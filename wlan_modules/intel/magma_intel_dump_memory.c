/*  © 2021 Edoardo Mantovani All Rights Reserved */
#include "magma_intel.h"

#define magma_shell_display_dump() {    \
                                        \
}   \

void magma_iwlwifi_dump_memory(){
    int i = 0x00000;
    struct magma_iwlwifi_io_res *magma_iwlwifi = (struct magma_iwlwifi_io_res *)kmalloc(sizeof(magma_iwlwifi), GFP_KERNEL);
    for(; i <= 0xFFFFFFFFFFF; i + 4 ){ /* 4 bytes of 'jump' */
        /* read 32 bits for time */
        magma_iwlwifi_io_res = magma_iwlwifi_main_io(MAGMA_IWLWIFI_DO_READ32, i, 0);
        usleep(400);
        magma_shell_display_dump(magma_iwlwifi_io_res->read32_res);
        
    }

    kfree(magma_iwlwifi);
}
