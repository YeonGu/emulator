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
    int      map_type;
    uint8_t *map_begin;
    addr_t   nes_begin;
    uint16_t map_size;

    addr_t mirror_to;

    void ( *mem_write_handler )( addr_t offset, uint8_t data );
};
void    init_ppu_memory_map();
uint8_t ppu_addr_read( addr_t addr );
void    ppu_addr_write( addr_t addr, uint8_t data );

enum
{
    HORIZON_SCREEN,
    VERTICAL_SCREEN,
};
enum
{
    TYPE_PHYSICAL,
    TYPE_MIRROR,
};

#endif // EMULATOR_PPU_MEMORY_H
