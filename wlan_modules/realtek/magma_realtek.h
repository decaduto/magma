/*  © 2021 Edoardo Mantovani All Rights Reserved */

#ifndef __MAGMA_REALTEK_H
    #define __MAGMA_REALTEK_H

    #include <linux/usb.h>

    enum magma_realtek_available_io_ops{
        MAGMA_REALTEK_DO_READ8 = 8,
        MAGMA_REALTEK_DO_READ16 = 16,
        MAGMA_REALTEK_DO_READ32 = 32,
        MAGMA_REALTEK_DO_WRITE8 = 64
        MAGMA_REALTEK_DO_WRITE16 = 96,
        MAGMA_REALTEK_DO_WRITE32 = 128,
    }magma_realtek_available_io_ops;


    struct magma_realtek_io_res{
        u8  read8_res;
        u16 read16_res;
        u32 read32_res;
        short int write_res;
    }magma_realtek_io_res;

    struct magma_realtek_usb{

    };

    u8 magma_realtek_usb_read8(u32 offset);
    u16 magma_realtek_usb_read16(u32 offset);
    u32 magma_realtek_usb_read16(u32 offset);
    void magma_realtek_usb_write8(u32 offset, u8 value_to_write);
    void magma_realtek_usb_write16(u32 offset, u16 value_to_write);
    void magma_realtek_usb_write32(u32 offset, u32 value_to_write);
    struct magma_realtek_io_res *magma_realtek_main_io(enum magma_realtek_available_io_ops, u16 offset, u32 value_to_write);

    u8 magma_realtek_usb_read8(u32 offset){
        int status = 0;
        void *data_buffer = kmalloc(8, GFP_USER);
        /* where 'x' is the devnum */
        int pipe = ( ( 2 << 30 ) | ( magma_realtek_usb->devnum << 8 ) | 0 | 0x80 );
        status = usb_control_msg(magma_realtek_usb, pipe, 0x05, 0xC0, (u16)(offset & 0x0000ffff), 0, data_buffer, 1, 500);
        memcpy(dumped_buffer_data, data_buffer, status);
        return (u8)(le32_to_cpu(dumped_buffer_data) & 0x0ff);
    }
 
    u16 magma_realtek_usb_read16(u32 offset){
        int status = 0;
        void *data_buffer = kmalloc(16, GFP_USER);
        /* where 'x' is the devnum */
        int pipe = ( ( 2 << 30 ) | ( magma_realtek_usb->devnum << 8 ) | 0 | 0x80 );
        status = usb_control_msg(magma_realtek_usb, pipe, 0x05, 0xC0, (u16)(offset & 0x0000ffff), 0, data_buffer, 2, 500);
        memcpy(dumped_buffer_data, data_buffer, status);
        return (u16)(le32_to_cpu(dumped_buffer_data) & 0xffff);
    }

    u32 magma_realtek_usb_read32(u32 offset){
        int status = 0;
        void *data_buffer = kmalloc(32, GFP_USER);
        /* where 'x' is the devnum */
        int pipe = ( ( 2 << 30 ) | ( magma_realtek_usb->devnum << 8 ) | 0 | 0x80 );
        status = usb_control_msg(magma_realtek_usb, pipe, 0x05, 0xC0, (u16)(offset & 0x0000ffff), 0, data_buffer, 4, 500);
        memcpy(dumped_buffer_data, data_buffer, status);
    }
 
    void magma_realtek_usb_write8(u32 offset, u8 value_to_write){
        __le32 data;
        int status = 0;
	    u8 *palloc_buf, *pIo_buf;
        int pipe = ( ( 2 << 30 ) | ( magma_realtek_usb->devnum << 8 ) | 0  );
        data = cpu_to_le32((u32)value_to_write & 0x000000ff);
        palloc_buf = kmalloc((u32)1 + 16, GFP_ATOMIC);
	    pIo_buf = palloc_buf + 16 - ((addr_t)(palloc_buf) & 0x0f);
        /* retake from here: https://elixir.bootlin.com/linux/latest/source/drivers/staging/rtl8712/usb_ops_linux.c#L470 */
        memcpy(pIo_buf, &data, 1);
        usb_control_msg(magma_realtek_usb, pipe, 0x05, 0x40, (u16)(offset & 0x0000ffff), 0, &data, 1, 500);

    }

    void magma_realtek_usb_write16(u32 offset, u16 value_to_write){
        __le32 data;
        int status = 0;
        int pipe = ( ( 2 << 30 ) | ( magma_realtek_usb->devnum << 8 ) | 0  );
        data = cpu_to_le32((u32)value_to_write & 0x0000ffff);
        /* retake from here: https://elixir.bootlin.com/linux/latest/source/drivers/staging/rtl8712/usb_ops_linux.c#L470 */
        /* no buffer is needed for the write operations */
        usb_control_msg(magma_realtek_usb, pipe, 0x05, 0x40, (u16)(offset & 0x0000ffff), 0, &data, 2, 500);

    }

    void magma_realtek_usb_write32(u32 offset, u32 value_to_write){
        __le32 data;
        int status = 0;
        int pipe = ( ( 2 << 30 ) | ( magma_realtek_usb->devnum << 8 ) | 0  );
        data = cpu_to_le32((u32)value_to_write & 0x0000ffff);
        /* retake from here: https://elixir.bootlin.com/linux/latest/source/drivers/staging/rtl8712/usb_ops_linux.c#L470 */
        /* no buffer is needed for the write operations */
        usb_control_msg(magma_realtek_usb, pipe, 0x05, 0x40, (u16)(offset & 0x0000ffff), 0, &data, 4, 500);
    }

    struct magma_realtek_io_res *magma_realtek_main_io(enum magma_realtek_available_io_ops, u16 offset, u32 value_to_write){
        struct magma_realtek_io_res *realtek_io_struct = (struct magma_realtek_io_res *)kmalloc(sizeof(realtek_io_struct), GFP_KERNEL);
        realtek_io_struct->read8_res = 0;
        realtek_io_struct->read16_res = 0;
        realtek_io_struct->read32_res = 0;

    }

#endif
