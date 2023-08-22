//
// Created by Gu Yuhang on 2023/8/21.
//

#ifndef EMULATOR_MEMORY_H
#define EMULATOR_MEMORY_H
#include "rom.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#define PRG_ROM_BLOCKS 64
#define CHR_ROM_BLOCKS 16

#define addr_t uint16_t

struct mem_map_t
{
    char    *name;
    uint8_t *begin;
    addr_t   map_base;
    uint16_t size;
};

void init_rom( FILE *file, struct nes_rom_info_t *info );
void init_mapper( struct nes_rom_info_t *info );

uint8_t vaddr_read( addr_t addr );

#endif // EMULATOR_MEMORY_H
