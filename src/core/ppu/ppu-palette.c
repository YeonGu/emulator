///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/29
//
//  ppu-palette.c
//
///////////////////////////////////////////////////////////////////////

#include <ppu-memory.h>
#include <ppu.h>
#include <stdint.h>

static uint8_t palette_ram_indexes[ 0x20 ];

static uint8_t universal_bg_color;
static uint8_t bg_palette[ 15 ];
static uint8_t sprite_palette[ 4 ][ 3 ];

void add_new_ppumap( char *name, int type, addr_t ppu_mem_addr, addr_t size, uint8_t *map_to );
void init_palette_memory( struct ppu_mem_map_t *map )
{
    add_new_ppumap( "universal_bg", TYPE_PHYSICAL, 0x3F00, 1, &universal_bg_color );
    add_new_ppumap( "bg_palette", TYPE_PHYSICAL, 0x3F01, 15, bg_palette );
    add_new_ppumap( "bg_mirror_0", TYPE_PHYSICAL, 0x3F10, 1, &universal_bg_color );

    add_new_ppumap( "sprite_palette_0", TYPE_PHYSICAL, 0x3F11, 3, sprite_palette[ 0 ] );
    add_new_ppumap( "bg_mirror_1", TYPE_PHYSICAL, 0x3F14, 1, bg_palette + 3 );
    add_new_ppumap( "sprite_palette_1", TYPE_PHYSICAL, 0x3F15, 3, sprite_palette[ 1 ] );
    add_new_ppumap( "bg_mirror_2", TYPE_PHYSICAL, 0x3F18, 1, bg_palette + 7 );
    add_new_ppumap( "sprite_palette_2", TYPE_PHYSICAL, 0x3F19, 3, sprite_palette[ 2 ] );
    add_new_ppumap( "bg_mirror_3", TYPE_PHYSICAL, 0x3F1C, 1, bg_palette + 11 );
    add_new_ppumap( "sprite_palette_3", TYPE_PHYSICAL, 0x3F1D, 3, sprite_palette[ 3 ] );
}
