///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/21
//
//  memory.c
//
///////////////////////////////////////////////////////////////////////

#include "memory.h"
#include "apu.h"
#include "input_manager.h"
#include "mmu.h"
#include "ppu.h"
#include <cstdio>
#include <cstdlib>

struct cpu_mem_map_t *find_cpu_map( addr_t addr );
APU                  *get_apu();

void init_memory()
{
    auto vmmu = get_mmu();
    // PPU IO
    vmmu->mmap(
        0x2000, nullptr, 0x3FFF - 0x2000 + 1,
        []( uint16_t addr ) -> uint16_t { return ppu_reg_read( addr % 8 ); },
        []( uint16_t addr, uint8_t data ) { ppu_reg_write( addr % 8, data ); } );
    // APU IO
    vmmu->mmap(
        0x4000, nullptr, 0x4013 - 0x4000 + 1,
        []( uint16_t addr ) -> auto { return get_apu()->apu_reg_read( addr ); },
        []( uint16_t addr, uint8_t data ) { get_apu()->apu_reg_write( addr, data ); } );
    // OAM DMA
    vmmu->mmap(
        0x4014, nullptr, 1,
        []( uint16_t addr ) -> uint16_t { return oamdma_read(); },
        []( uint16_t addr, uint8_t data ) { oamdma_write( data ); } );
    // APU IO
    vmmu->mmap(
        0x4015, nullptr, 1,
        []( uint16_t addr ) -> auto { return get_apu()->apu_reg_read( addr ); },
        []( uint16_t addr, uint8_t data ) { get_apu()->apu_reg_write( addr, data ); } );
    vmmu->mmap(
        0x4017, nullptr, 1,
        []( uint16_t addr ) -> auto { return get_apu()->apu_reg_read( addr ); },
        []( uint16_t addr, uint8_t data ) { get_apu()->apu_reg_write( addr, data ); } );
}

uint8_t vaddr_read( addr_t addr )
{
    //    if ( addr <= 0x2007 && addr >= 0x2000 )
    //        printf( "vaddr read %04x\n", addr );
    return get_mmu()[ 0 ][ addr ];
}

void vaddr_write( addr_t addr, uint8_t data )
{
    //    if ( addr <= 0x2007 && addr >= 0x2000 )
    //        printf( "vaddr write %04x\n", addr );
    get_mmu()[ 0 ][ addr ] = data;
}
