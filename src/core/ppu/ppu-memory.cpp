///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/28
//
//  ppu-memory.c
//
///////////////////////////////////////////////////////////////////////
#include "ppu-memory.h"

static uint8_t ciram_0[ 0x0400 ]; // vram / nametable
static uint8_t ciram_1[ 0x0400 ];

static struct ppu_mem_map_t ppu_memory_map[ 32 ];
static int                  nr_ppu_map;

struct ppu_mem_map_t *find_ppu_map( addr_t addr );
uint8_t               ppu_addr_read( addr_t addr )
{
    struct ppu_mem_map_t *map = find_ppu_map( addr );
    assert( map && ( map->map_type == TYPE_PHYSICAL ) );

    addr_t offset = addr - map->nes_begin;
    return map->map_begin[ offset ];
}
void ppu_addr_write( addr_t addr, uint8_t data )
{
    struct ppu_mem_map_t *map = find_ppu_map( addr );
    assert( map && ( map->map_type == TYPE_PHYSICAL ) );
    addr_t offset            = addr - map->nes_begin;
    map->map_begin[ offset ] = data;
}

extern struct nes_rom_info_t rom_info;
uint8_t                     *get_chr_rom( int idx );
#define NEW_MAP( idx_, name_, type_, addr_, size_, map_ ) \
    ppu_memory_map[ nr_ppu_map ].map_name  = name_;       \
    ppu_memory_map[ nr_ppu_map ].nes_begin = addr_;       \
    ppu_memory_map[ nr_ppu_map ].map_size  = size_;       \
    ppu_memory_map[ nr_ppu_map ].map_begin = map_;        \
    ppu_memory_map[ nr_ppu_map ].map_type  = type_;       \
    nr_ppu_map++;
void add_new_ppumap( char *name, int type, addr_t ppu_mem_addr, addr_t size, uint8_t *map_to )
{
    ppu_memory_map[ nr_ppu_map ].map_name  = name;
    ppu_memory_map[ nr_ppu_map ].nes_begin = ppu_mem_addr;
    ppu_memory_map[ nr_ppu_map ].map_size  = size;
    ppu_memory_map[ nr_ppu_map ].map_begin = map_to;
    ppu_memory_map[ nr_ppu_map ].map_type  = type;
    nr_ppu_map++;
}
void init_ppu_nametable_map( int mirtype );
void init_palette_memory( struct ppu_mem_map_t *map, int *nr_ppu_map );
void init_ppu_memory_map()
{
    NEW_MAP( 0, "pattern_table_0", TYPE_PHYSICAL, 0x0000, 0x1000, get_chr_rom( 0 ) );
    NEW_MAP( 1, "pattern_table_1", TYPE_PHYSICAL, 0x1000, 0x1000, get_chr_rom( 0 ) + 0x1000 );

    // 4 Nametable Mirrors.
    // TODO: more mirror type...
    if ( rom_info.nametable_mirror_type == 0 ) // 0: Horizontal (vertical arrangement) or mapper-controlled | 1: Vertical (horizontal arrangement)
    {
        switch ( rom_info.mapper )
        {
        case 0:
            init_ppu_nametable_map( HORIZON_SCREEN );
            break;
        default:
            assert( 0 );
        }
    }
    else
    {
        init_ppu_nametable_map( VERTICAL_SCREEN );
    }

    NEW_MAP( 6, "nametable_mirror", TYPE_MIRROR, 0x3000, 0x0F00, NULL );
    ppu_memory_map[ nr_ppu_map - 1 ].mirror_to = 0x2000;

    //    NEW_MAP( 7, "palette_ram_indexes", TYPE_PHYSICAL, 0x3F00, 0x0020, palette_ram_indexes );
    init_palette_memory( ppu_memory_map, &nr_ppu_map );
    NEW_MAP( 8, "palette_mirror", TYPE_MIRROR, 0x3F20, 0x0020, NULL );
    ppu_memory_map[ nr_ppu_map - 1 ].mirror_to = 0x3F00;
}

void init_ppu_nametable_map( int mirtype )
{
    if ( mirtype == HORIZON_SCREEN )
    {
        NEW_MAP( 2, "nametable_0", TYPE_PHYSICAL, 0x2000, 0x0400, ciram_0 );
        NEW_MAP( 3, "nametable_1", TYPE_PHYSICAL, 0x2400, 0x0400, ciram_1 );
        NEW_MAP( 4, "nametable_2", TYPE_PHYSICAL, 0x2800, 0x0400, ciram_0 );
        NEW_MAP( 5, "nametable_3", TYPE_PHYSICAL, 0x2C00, 0x0400, ciram_1 );
    }
    else if ( mirtype == VERTICAL_SCREEN )
    {
        NEW_MAP( 2, "nametable_0", TYPE_PHYSICAL, 0x2000, 0x0400, ciram_0 );
        NEW_MAP( 3, "nametable_1", TYPE_PHYSICAL, 0x2400, 0x0400, ciram_0 );
        NEW_MAP( 4, "nametable_2", TYPE_PHYSICAL, 0x2800, 0x0400, ciram_1 );
        NEW_MAP( 5, "nametable_3", TYPE_PHYSICAL, 0x2C00, 0x0400, ciram_1 );
    }
}

struct ppu_mem_map_t *find_ppu_map( addr_t addr )
{
    int i;
    for ( i = nr_ppu_map - 1; i >= 0; i-- )
    {
        if ( ppu_memory_map[ i ].nes_begin <= addr )
            break;
    }
    if ( ppu_memory_map[ i ].map_type == TYPE_PHYSICAL )
    {
        //        addr_t offset = addr - ppu_memory_map[ i ].nes_begin;
        //        assert( offset < ppu_memory_map[ i ].map_size );
        return &ppu_memory_map[ i ];
    }
    else if ( ppu_memory_map[ i ].map_type == TYPE_MIRROR )
    {
        addr_t offset = ( addr - ppu_memory_map[ i ].nes_begin ) % ppu_memory_map[ i ].map_size;
        return find_ppu_map( ppu_memory_map[ i ].mirror_to + offset );
    }
    return NULL;
}
