///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/21
//
//  memory.c
//
///////////////////////////////////////////////////////////////////////

#include "memory.h"

static uint8_t prg_rom[PRG_ROM_BLOCKS * 16 * 1024];
static uint8_t chr_rom[CHR_ROM_BLOCKS * 16 * 1024];


