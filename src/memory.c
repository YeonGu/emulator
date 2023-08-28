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

struct cpu_mem_map_t *find_cpu_map( addr_t addr );
uint8_t               vaddr_read( addr_t addr )
{
    struct cpu_mem_map_t *map = find_cpu_map( addr );
    assert( map );
    addr_t offset = addr - map->nes_begin;

    if ( map->mem_read_handler ) map->mem_read_handler( offset );
    //    if ( ( addr <= 0x2007 && addr >= 0x2000 ) || addr == 0x4014 )
    //    {
    //    printf( "\tMTRACE: READ %02x at %04x\n", map->map_begin[ offset ], addr );
    //    }
    uint8_t res = map->map_begin[ offset ];

    return res;
}

void vaddr_write( addr_t addr, uint8_t data )
{
    struct cpu_mem_map_t *map = find_cpu_map( addr );
    //    assert( map );

    addr_t offset = addr - map->nes_begin;
    if ( ( addr <= 0x2007 && addr >= 0x2000 ) || addr == 0x4014 )
    {
        printf( "\tMTRACE: WRITE %02x at %04x (%s)\n", data, addr, map->map_name );
    }

    map->map_begin[ offset ] = data;
    if ( map->mem_write_handler ) map->mem_write_handler( offset, data );
}
