///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/29
//
//  render.c
//
///////////////////////////////////////////////////////////////////////

#include <stdint.h>

uint32_t get_bg_palette_color( uint8_t index );

uint32_t get_bg_color( int x, int y )
{
    // FIXME: scroll, other palette
    int tile_x = x / 8;
    int tile_y = y / 8; // Nametable location
}
