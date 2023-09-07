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

byte        ppu_iobus_value; // https://www.nesdev.org/wiki/Open_bus_behavior
static byte data_latch;

////////////////////////////////////////////////////////////////////////////////////////
// PPU internal regs. see: PPU scroll (From ppu.h)
// addr_t vram_addr.data    = 0;
// addr_t tmp_addr.data     = 0;
// byte   finex_scroll = 0;
// bool   write_toggle = false;

void ppu::init_io_register_handlers()
{
    // $2000 CTRL > write
    ppu_io_reg[ PPUREG_CTRL ].write_handler = [ this ]( byte data ) -> void {
        //        tmp_addr.data &= 0x3FF;
        //        tmp_addr.data |= data & 0x3 << 10;
        ppu_io_reg[ PPUREG_CTRL ].reg_data = data;
        tmp_addr.nametable_select          = data & 0x3;
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
    ppu_io_reg[ PPUREG_STATUS ].read_handler = [ this ]() -> byte {
        auto data = ( ppu_io_reg[ PPUREG_STATUS ].reg_data & 0xE0 ) | ( data_latch & 0x1F );

        reinterpret_cast<ppustatus_flag_t &>( ppu_io_reg[ PPUREG_STATUS ].reg_data ).nmi_flag = 0;
        return data;
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
        if ( !write_toggle ) // $2005 first write
        {
            ppu_io_reg[ PPUREG_SCROLL ].reg_data = data;
            write_toggle                         = true;
            finex_scroll                         = data & 0x7;
            //            tmp_addr.data &= 0x1F;
            //            tmp_addr.data |= ( data >> 3 );
            tmp_addr.coarse_x = data >> 3;
        }
        else // second write
        {
            ppu_io_reg[ PPUREG_SCROLL ].reg_data = data;
            write_toggle                         = false;
            //            tmp_addr.data &= 0xC1F;
            //            tmp_addr.data |= data & 0x7 << 12;
            //            tmp_addr.data |= data >> 3 << 5;
            tmp_addr.coarse_y_l = data >> 3;
            tmp_addr.coarse_y_h = data >> 6;
            tmp_addr.fine_y     = data & 0x7;
        }
    };
    ppu_io_reg[ PPUREG_SCROLL ].read_handler = [ this ]() -> byte {
        return ppu_iobus_value;
    };

    // $2006 ADDRESS >> write x2
    ppu_io_reg[ PPUREG_ADDR ].write_handler = [ this ]( byte data ) {
        if ( !write_toggle )
        {
            ppu_io_reg[ PPUREG_ADDR ].reg_data = data;
            write_toggle                       = true;
            tmp_addr.data                      = 0;
            tmp_addr.data |= (addr_t) ( data & 0x3F ) << 8;
        }
        else
        {
            write_toggle                       = false;
            ppu_io_reg[ PPUREG_ADDR ].reg_data = data;
            tmp_addr.data &= 0xFF00;
            tmp_addr.data |= data;
            vram_addr.data = tmp_addr.data;
        }
    };
    ppu_io_reg[ PPUREG_ADDR ].read_handler = [ this ]() -> byte {
        return ppu_iobus_value;
    };

    // $2007
    ppu_io_reg[ PPUREG_DATA ].write_handler = [ this ]( byte data ) {
        mwrite( vram_addr.data, data );
        vram_addr.data += reinterpret_cast<ppuctrl_flag_t &>( ppu_io_reg[ PPUREG_CTRL ].reg_data ).vram_inc ? 32 : 1;
    };
    ppu_io_reg[ PPUREG_DATA ].read_handler = [ this ]() {
        addr_t addr = vram_addr.data % 0x4000;
        byte   data;

        if ( addr >= 0x3F00 )
        {
            data       = mread( addr );
            data_latch = mread( addr - 0x1F00 );
        }
        else
        {
            data = mread( addr );
            std::swap( data_latch, data );
        }
        vram_addr.data += reinterpret_cast<ppuctrl_flag_t &>( ppu_io_reg[ PPUREG_CTRL ].reg_data ).vram_inc ? 32 : 1;
        return data;
    };
}
