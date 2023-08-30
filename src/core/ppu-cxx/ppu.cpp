//
// Created by 顾雨杭 on 2023/8/30.
//

#include "ppu.h"
#include <cstdlib>
#include <cstring>

ppu::ppu( uint8_t *chr_rom, int screen_arrangement )
    : scr_arrange( screen_arrangement )
{
    memcpy( pattern_table_0, chr_rom, sizeof( pattern_table_0 ) );
    memcpy( pattern_table_1, chr_rom + sizeof( pattern_table_0 ), sizeof( pattern_table_0 ) );
    ppu_inst = this;

    mmap.emplace_back<mem_map_t>( { 0x0000, 0x1000, pattern_table_0 } );
    mmap.emplace_back<mem_map_t>( { 0x1000, 0x1000, pattern_table_1 } );
    if ( screen_arrangement == HORIZON_SCREEN )
    {
        mmap.emplace_back<mem_map_t>( { 0x2000, 0x0400, ciram_0 } );
        mmap.emplace_back<mem_map_t>( { 0x2400, 0x0400, ciram_1 } );
        mmap.emplace_back<mem_map_t>( { 0x2800, 0x0400, ciram_0 } );
        mmap.emplace_back<mem_map_t>( { 0x2C00, 0x0400, ciram_1 } );
    }
    else if ( screen_arrangement == VERTICAL_SCREEN )
    {
        mmap.emplace_back<mem_map_t>( { 0x2000, 0x0400, ciram_0 } );
        mmap.emplace_back<mem_map_t>( { 0x2400, 0x0400, ciram_0 } );
        mmap.emplace_back<mem_map_t>( { 0x2800, 0x0400, ciram_1 } );
        mmap.emplace_back<mem_map_t>( { 0x2C00, 0x0400, ciram_1 } );
    }
    mmap.emplace_back<mem_map_t>( { 0x3000, 0x0F00, nullptr, true, 0x2000 } );

    // $3F00 - Palette RAM
    palette_map[ 0 ] = &uni_bg_color;
    for ( int i = 1; i <= 0x0F; ++i )
    {
        palette_map[ i ] = bg_palette + i;
    }
    palette_map[ 0x10 ] = &uni_bg_color;
    palette_map[ 0x11 ] = sp_palette[ 0 ];
    palette_map[ 0x12 ] = sp_palette[ 0 ] + 1;
    palette_map[ 0x13 ] = sp_palette[ 0 ] + 2;
    palette_map[ 0x14 ] = palette_map[ 0x04 ];
    palette_map[ 0x15 ] = sp_palette[ 1 ];
    palette_map[ 0x16 ] = sp_palette[ 1 ] + 1;
    palette_map[ 0x17 ] = sp_palette[ 1 ] + 2;
    palette_map[ 0x18 ] = palette_map[ 0x08 ];
    palette_map[ 0x19 ] = sp_palette[ 2 ];
    palette_map[ 0x1A ] = sp_palette[ 2 ] + 1;
    palette_map[ 0x1B ] = sp_palette[ 2 ] + 2;
    palette_map[ 0x1C ] = palette_map[ 0x0C ];
    palette_map[ 0x1D ] = sp_palette[ 3 ];
    palette_map[ 0x1E ] = sp_palette[ 3 ] + 1;
    palette_map[ 0x1F ] = sp_palette[ 3 ] + 2;
}

uint8_t ppu::mread( addr_t addr ) // 3F00 split search
{
}

uint8_t ppu::mwrite( addr_t addr, byte data )
{
}

void ppu::render_screen( uint32_t *vmem )
{
}

void ppu_reg_write( int idx, byte data )
{
    assert( idx <= 7 );
    switch ( idx )
    {
    case PPUREG_ADDR:
        ppu_inst->get_reg( idx ) = data;
        static addr_t tmp_addr;
        if ( ppu_inst->vaddr_wr_h )
            tmp_addr = tmp_addr & 0x00FF | ( data << 8 ); // Set higher bits in tmp
        else
        {
            tmp_addr            = tmp_addr & 0xFF00 | data; // Set lower bits in tmp, set the vaddr
            ppu_inst->vram_addr = tmp_addr;
        }
        ppu_inst->vaddr_wr_h = !ppu_inst->vaddr_wr_h;
        break;

    case PPUREG_DATA: // write to ppudata (0x7)

    default:
        ppu_inst->get_reg( idx ) = data;
        break;
    }
}

byte ppu_reg_read( int idx )
{
    assert( idx <= 7 );
    if ( idx != PPUREG_DATA )
        return ppu_inst->get_reg( idx );

    static byte readdata_buf;
    // Read PPUDATA
    byte data = ppu_inst->mread( ppu_inst->vram_addr );
    ppu_inst->vram_addr += ( get_vram_inc() ) ? 1 : 32;

    if ( ppu_inst->vram_addr < 0x3F00 )
    {
        std::swap( data, readdata_buf );
    }
    return data;
}
