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
#define NEW_MAP( i, name, addr, size, dst ) \
    mapper[ i ].map_name  = name;           \
    mapper[ i ].nes_begin = addr;           \
    mapper[ i ].map_size  = size;           \
    mapper[ i ].map_begin = dst;
void init_mapper( struct nes_rom_info_t *info )
{
    for ( int i = 0; i < 4; ++i )
    {
        NEW_MAP( i, "ram", 0x0800 * i, 0x800, ram );
    }

    NEW_MAP( 4, "ppu_registers", 0x2000, 0x0008, ppu_reg );
    NEW_MAP( 5, "apu_io_registers", 0x4000, 0x0018, apu_io_reg );

    // TODO: other mapper?
    // The cartridge space at $4020-$FFFF can be used by cartridges for any purpose,
    // such as ROM, RAM, and registers. Many common mappers place ROM and save/work RAM in these locations:
    //      $6000–$7FFF: Battery-backed save or work RAM
    //      $8000–$FFFF: ROM and mapper registers (see MMC1 and UxROM for examples)
    NEW_MAP( 6, "sram", 0x6000, 0x2000, sram );

    switch ( info->mapper )
    {
    case 0: // https://www.nesdev.org/wiki/NROM
        NEW_MAP( 7, "prg_rom_0", 0x8000, 0x4000, prg_rom_0 );
        NEW_MAP( 8, "prg_rom_1", 0xC000, 0x4000, ( info->prg_size == 2 ) ? prg_rom_1 : prg_rom_0 );
        break;

    default:
        printf( "MAPPER %d NOT IMPLEMENTED\n", info->mapper );
        assert( 0 );
    }

    printf("Mapper init finished.\n");
}