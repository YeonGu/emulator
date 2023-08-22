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

//void init_prg( FILE *file, long prg_blocks );
//void init_chr( FILE *file, long chr_blocks );
void init_rom( FILE *file, struct nes_rom_info_t *info );

void init_mapper( struct nes_rom_info_t *info );

#endif // EMULATOR_MEMORY_H
