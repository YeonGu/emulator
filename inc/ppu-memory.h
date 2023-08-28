//
// Created by 顾雨杭 on 2023/8/28.
//

#ifndef EMULATOR_PPU_MEMORY_H
#define EMULATOR_PPU_MEMORY_H

#include "memory.h"
#include <stdint.h>

struct ppu_mem_map_t
{
    char    *map_name;
    uint8_t *map_begin;
    addr_t   nes_begin;
    uint16_t map_size;

    void ( *mem_write_handler )( addr_t offset, uint8_t data );
};

#endif // EMULATOR_PPU_MEMORY_H
