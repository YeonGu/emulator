///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/21
//
//  memory.c
//
///////////////////////////////////////////////////////////////////////

#include "memory.h"
#include <stdio.h>
#include <stdlib.h>

struct mem_map_t *find_map( addr_t addr );
uint8_t           vaddr_read( addr_t addr )
{
    struct mem_map_t *map = find_map( addr );
    assert( map );
    addr_t offset = addr - map->nes_begin;

    //    if ( ( addr <= 0x2007 && addr >= 0x2000 ) || addr == 0x4014 )
    //    {
    //    printf( "\tMTRACE: READ %02x at %04x\n", map->map_begin[ offset ], addr );
    //    }
    if ( map->mem_prev_handler ) map->mem_prev_handler( addr );
    uint8_t res = map->map_begin[ offset ];
    if ( map->mem_after_handler ) map->mem_after_handler( addr );

    return res;
}

void vaddr_write( addr_t addr, uint8_t data )
{
    struct mem_map_t *map = find_map( addr );
    assert( map );
    addr_t offset = addr - map->nes_begin;

    if ( ( addr <= 0x2007 && addr >= 0x2000 ) || addr == 0x4014 )
    {
        printf( "\tMTRACE: WRITE %02x at %04x\n", data, addr );
    }

    map->map_begin[ offset ] = data;
}
