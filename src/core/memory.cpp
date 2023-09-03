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
#include "input_manager.h"
#include <cstdio>
#include <cstdlib>

struct cpu_mem_map_t *find_cpu_map( addr_t addr );

void init_memory()
{
    auto vmmu = get_mmu();
    //PPU MEM
    vmmu->mmap(0x2000, nullptr,0x3FFF - 0x2000 + 1,[](uint16_t addr) -> uint16_t{
        return ppu_reg_read( addr % 8 );
    }, [](uint16_t addr,uint8_t data){
        ppu_reg_write( addr % 8, data );
    });
    //OAM DMA
    vmmu->mmap(0x4014, nullptr,1,[](uint16_t addr) -> uint16_t{
        return oamdma_read();
    }, [](uint16_t addr,uint8_t data){
        oamdma_write( data );
    });
    //gamepad
    vmmu->mmap(0x4016, nullptr,1,get_key_shift_reg_lsb,ready_to_get_shift_reg);
}

uint8_t vaddr_read( addr_t addr )
{

    return (*get_mmu())[addr];
}

void vaddr_write( addr_t addr, uint8_t data )
{
    (*get_mmu())[addr] = data;
}
