//
// Created by Gu Yuhang on 2023/8/21.
//

#ifndef EMULATOR_MEMORY_H
#define EMULATOR_MEMORY_H
#include <stdint.h>

#define PRG_ROM_BLOCKS 64
#define CHR_ROM_BLOCKS 16

#define addr_t uint16_t

void init_prg();
void init_chr();

#endif // EMULATOR_MEMORY_H
