///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/21
//
//  memory.c
//
///////////////////////////////////////////////////////////////////////

#include "memory.h"

static uint8_t prg_rom[ PRG_ROM_BLOCKS * 16 * 1024 ];
static uint8_t chr_rom[ CHR_ROM_BLOCKS * 16 * 1024 ];
static long prg_size;
static long chr_size;

// TODO: init prg/chr fault process
void init_prg( FILE *file, long size )
{
    assert(fread( prg_rom, size, 1, file ) == 1);
    printf("TiNES init PRG, size = %ld bytes.\n", size);
    prg_size = size;
}
void init_chr( FILE *file, long size )
{
    fread( chr_rom, size, 1, file );
    printf("TiNES init CHR, size = %ld bytes.\n", size);
    chr_size = size;
}
