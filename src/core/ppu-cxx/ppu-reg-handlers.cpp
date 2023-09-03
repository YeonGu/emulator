///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/9/2
//
//  ppu-reg-handlers.cpp
//
///////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <ppu.h>
using reg_read_behavior  = std::function<byte()>;
using reg_write_behavior = std::function<void( byte )>;
byte ppu_iobus_value; // https://www.nesdev.org/wiki/Open_bus_behavior

void ppu::init_io_register_handlers()
{
    // $2000 CTRL > write
    ppu_io_reg[ PPUREG_CTRL ].write_handler = [ this ]( byte data ) -> void {
        tmp_addr &= 0x3FF;
        tmp_addr |= data & 0x3 << 10;
        ppu_io_reg[ PPUREG_CTRL ].reg_data = data;
    };
    ppu_io_reg[ PPUREG_CTRL ].read_handler = [ this ]() -> byte {
        return ppu_iobus_value;
    };

    // $2001 MASK > write
    ppu_io_reg[ PPUREG_MASK ].read_handler = [ this ]() -> byte {
        return ppu_iobus_value;
    };

    // $2002
    ppu_io_reg[ PPUREG_STATUS ].write_handler = [ this ]( byte data ) {
        write_toggle                         = false;
        ppu_io_reg[ PPUREG_STATUS ].reg_data = data;
    };

    // $2003
    ppu_io_reg[ PPUREG_OAMADDR ].write_handler = [ this ]( byte data ) {
        ppu_io_reg[ PPUREG_OAMADDR ].reg_data = data;
    };
    ppu_io_reg[ PPUREG_OAMADDR ].read_handler = [ this ]() -> byte {
        return ppu_iobus_value;
    };
    // $2005 SCROLL >> write x2
    ppu_io_reg[ PPUREG_SCROLL ].write_handler = [ this ]( byte data ) {
        if ( !write_toggle )
        { // $2005 first write
            write_toggle                         = true;
            ppu_io_reg[ PPUREG_SCROLL ].reg_data = data;
            tmp_addr &= 0x1F;
            tmp_addr |= ( data >> 3 );
            finex_scroll = data & 0x7;
        }
        else // second write
        {
            write_toggle                         = false;
            ppu_io_reg[ PPUREG_SCROLL ].reg_data = data;
            tmp_addr &= 0xC1F;
            tmp_addr |= data & 0x7 << 12;
            tmp_addr |= data >> 3 << 5;
        }
    };
    ppu_io_reg[ PPUREG_SCROLL ].read_handler = [ this ]() -> byte {
        return ppu_iobus_value;
    };

    // $2006 ADDRESS >> write x2
    ppu_io_reg[ PPUREG_ADDR ].write_handler = [ this ]( byte data ) {
        if ( !write_toggle )
        {
            write_toggle                       = true;
            ppu_io_reg[ PPUREG_ADDR ].reg_data = data;
            tmp_addr                           = 0;
            tmp_addr |= (addr_t) ( data & 0x3F ) << 8;
        }
        else
        {
            write_toggle                       = false;
            ppu_io_reg[ PPUREG_ADDR ].reg_data = data;
            tmp_addr &= 0xFF00;
            tmp_addr |= data;
            vram_addr = tmp_addr;

            //            printf( "write vram addr = %04x\n", vram_addr );
        }
    };
    ppu_io_reg[ PPUREG_ADDR ].read_handler = [ this ]() -> byte {
        return ppu_iobus_value;
    };

    // $2007
    ppu_io_reg[ PPUREG_DATA ].write_handler = [ this ]( byte data ) {
        //        if ( vram_addr < 0x2060 ) printf( "write %02x to reg 7, vram = %4x\n", data, vram_addr );
        mwrite( vram_addr, data );
        vram_addr += reinterpret_cast<ppuctrl_flag_t &>( ppu_io_reg[ PPUREG_CTRL ].reg_data ).vram_inc ? 32 : 1;
    };
    ppu_io_reg[ PPUREG_DATA ].read_handler = [ this ]() {
        //        printf( "read from reg 7, vram = %4x\n", vram_addr );
        static byte data_latch;
        auto        item = mread( vram_addr );
        std::swap( data_latch, item );
        return item;
    };
}
