///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/21
//
//  memory.c
//
///////////////////////////////////////////////////////////////////////

#include "memory.h"
#include "ppu.h"
#include <cstdio>
#include <cstdlib>

struct cpu_mem_map_t *find_cpu_map( addr_t addr );

uint8_t vaddr_read( addr_t addr )
{
    if ( ( addr >= 0x2000 ) && ( addr <= 0x3FFF ) )
        return ppu_reg_read( addr % 8 );
    if ( addr == 0x4014 )
        return oamdma_read();

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
    if ( ( addr >= 0x2000 ) && ( addr <= 0x3FFF ) )
    {
        ppu_reg_write( addr % 8, data );
        return;
    }
    if ( addr == 0x4014 )
    {
        oamdma_write( data );
        return;
    }
    struct cpu_mem_map_t *map = find_cpu_map( addr );

    addr_t offset = addr - map->nes_begin;
    //    if ( ( addr <= 0x2007 && addr >= 0x2000 ) || addr == 0x4014 )
    //    {
    //        printf( "\tMTRACE: WRITE %02x at %04x (%s)\n", data, addr, map->map_name );
    //    }

    map->map_begin[ offset ] = data;
    if ( map->mem_write_handler ) map->mem_write_handler( offset, data );
}
