///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/21
//
//  memory.c
//
///////////////////////////////////////////////////////////////////////

#include "memory.h"
#include "input_manager.h"
#include "mmu.h"
#include "ppu.h"
#include <cstdio>
#include <cstdlib>

struct cpu_mem_map_t *find_cpu_map( addr_t addr );

void init_memory()
{
    auto vmmu = get_mmu();
    // PPU MEM
    vmmu->mmap(
        0x2000, nullptr, 0x3FFF - 0x2000 + 1,
        []( uint16_t addr ) -> uint16_t { return ppu_reg_read( addr % 8 ); },
        []( uint16_t addr, uint8_t data ) { ppu_reg_write( addr % 8, data ); } );
    // OAM DMA
    vmmu->mmap(
        0x4014, nullptr, 1,
        []( uint16_t addr ) -> uint16_t { return oamdma_read(); },
        []( uint16_t addr, uint8_t data ) { oamdma_write( data ); } );
}

uint8_t vaddr_read( addr_t addr )
{
    //    if ( addr <= 0x2007 && addr >= 0x2000 )
    //        printf( "vaddr read %04x\n", addr );
    return get_mmu()[0][ addr ];
}

void vaddr_write( addr_t addr, uint8_t data )
{
    //    if ( addr <= 0x2007 && addr >= 0x2000 )
    //        printf( "vaddr write %04x\n", addr );
    get_mmu()[0][ addr ] = data;
}
