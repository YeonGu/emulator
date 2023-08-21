//
// Created by Gu Yuhang on 2023/8/21.
//

#ifndef EMULATOR_MEMORY_H
#define EMULATOR_MEMORY_H
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#define PRG_ROM_BLOCKS 64
#define CHR_ROM_BLOCKS 16

#define addr_t uint16_t

void init_prg( FILE *file , long size);
void init_chr( FILE *file , long size);

#endif // EMULATOR_MEMORY_H
