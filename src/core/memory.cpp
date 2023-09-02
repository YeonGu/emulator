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
#include "mmu.h"
#include <cstdio>
#include <cstdlib>

struct cpu_mem_map_t *find_cpu_map( addr_t addr );

void init_memory()
{
    auto vmmu = get_mmu();
    vmmu->mmap(0x0,NULL,0x10000,[](uint16_t addr){
        struct cpu_mem_map_t *map = find_cpu_map( addr );
        assert( map );
        addr_t offset = addr - map->nes_begin;

        if ( map->mem_read_handler ) map->mem_read_handler( offset );
        uint8_t res = map->map_begin[ offset ];

        return res;
    }, [](uint16_t addr,uint8_t data){
        struct cpu_mem_map_t *map = find_cpu_map( addr );
        addr_t offset = addr - map->nes_begin;
        map->map_begin[ offset ] = data;
        if ( map->mem_write_handler ) map->mem_write_handler( offset, data );
    });
    //PPU MEM
    vmmu->mmap(0x2000,NULL,0x3FFF - 0x2000 + 1,[](uint16_t addr) -> uint16_t{
        return ppu_reg_read( addr % 8 );
    }, [](uint16_t addr,uint8_t data){
        ppu_reg_write( addr % 8, data );
    });
    //OAM DMA
    vmmu->mmap(0x4014,NULL,1,[](uint16_t addr) -> uint16_t{
        return oamdma_read();
    }, [](uint16_t addr,uint8_t data){
        oamdma_write( data );
    });
}

uint8_t vaddr_read( addr_t addr )
{

    return (*get_mmu())[addr];
}

void vaddr_write( addr_t addr, uint8_t data )
{
    (*get_mmu())[addr] = data;
}
