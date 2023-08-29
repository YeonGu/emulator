///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/29
//
//  render.c
//
///////////////////////////////////////////////////////////////////////

#include <memory.h>
#include <ppu-memory.h>
#include <stdint.h>

uint32_t get_bg_palette_color( uint8_t index );

uint32_t get_bg_color( int x, int y )
{
    // FIXME: scroll, other palette
    int tile_x = x / 8;
    int tile_y = y / 8; // Nametable location
    int fine_x = x % 8;
    int fine_y = y % 8;

    addr_t bg_nt_base = 0x2000;

    addr_t pat_addr = ppu_addr_read( tile_y * 32 + tile_x + bg_nt_base ); // Pattern Table index
    pat_addr <<= 4;
    pat_addr |= fine_y; // Pattern Table row. TODO: another pat table for bg
                        //    pat_addr |= 0x8;
    // TODO: flip?
    uint8_t pattern_l = ppu_addr_read( pat_addr ) >> ( 7 - fine_x ) & 1;
    uint8_t pattern_h = ppu_addr_read( pat_addr + 8 ) >> ( 7 - fine_x ) & 1;
    uint8_t plte_idx  = ( pattern_h << 1 ) | pattern_l; // The Palette index for this pixel

    // Get the palette section in Attr. table
    int atr_x   = x / 32;
    int atr_y   = y / 32;
    fine_x      = x % 32 / 16;
    fine_y      = y % 32 / 16;
    int bit_off = fine_x * 2 + fine_y * 4; // Get Dx for palette index

    int atr_data     = vaddr_read( atr_y * 8 + atr_x );
    int plte_section = atr_data >> bit_off & 0b11;

    return get_bg_palette_color( plte_section * 4 + plte_idx );
}

void render_bg( uint32_t *vmem )
{
    for ( int y = 0; y < 240; ++y )
    {
        for ( int x = 0; x < 256; ++x )
        {
            *( vmem + x + y * 256 ) = get_bg_color( x, y );
        }
    }
}