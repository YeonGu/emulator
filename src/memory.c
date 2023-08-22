///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/21
//
//  memory.c
//
///////////////////////////////////////////////////////////////////////

#include "memory.h"

static uint8_t ram[ 0x0800 ];
static uint8_t ppu_reg[ 0x0008 ];
static uint8_t apu_io_reg[ 0x0018 ];

static uint8_t sram[ 0x2000 ];

static uint8_t prg_rom_0[ 1 * 16 * 1024 ];
static uint8_t prg_rom_1[ 1 * 16 * 1024 ];
static uint8_t chr_rom[ 1 * 8 * 1024 ];
static long    prg_size;
static long    chr_size;

struct mem_map_t mapper[ 10 ];

// TODO: init prg/chr fault process
void init_rom( FILE *file, struct nes_rom_info_t *info )
{
    if ( info->mapper != 0 )
    {
        printf( "ERROR: mapper = %d, NOT IMPLEMENTED.", info->mapper );
        assert( 0 );
    }
    assert( ( info->prg_size == 1 ) || ( info->prg_size == 2 ) );

    assert( fread( prg_rom_0, sizeof( prg_rom_0 ), 1, file ) == 1 );
    if ( info->prg_size == 2 )
    {
        assert( fread( prg_rom_1, sizeof( prg_rom_1 ), 1, file ) == 1 );
    }

    printf( "\nTiNES init PRG, prg_blocks = %d.\n", info->prg_size );
    prg_size = info->prg_size;

    if ( !info->chr_size )
        return;
    fread( chr_rom, sizeof( chr_rom ), 1, file );
    printf( "TiNES init CHR, chr_blocks = %d.\n", info->chr_size );
    chr_size = info->chr_size;
}

/**
 * https://www.nesdev.org/wiki/CPU_memory_map
 * */
void init_mapper( struct nes_rom_info_t *info )
{
    // TODO: other mapper?
}