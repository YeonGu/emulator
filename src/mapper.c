///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/27
//
//  mapper.c
//
///////////////////////////////////////////////////////////////////////

#include "ppu-reg.h"
#include <memory.h>

struct mem_map_t mapper[ 10 ];
int              nr_mapper;
addr_t           RESET_VECTOR, NMI_VECTOR, IRQ_BRK_VECTOR;

static uint8_t ram[ 0x0800 ];
// static uint8_t ppu_reg[ 0x0008 ];
static uint8_t apu_io_reg[ 0x0018 ];
static uint8_t sram[ 0x2000 ];

uint8_t *get_prg_rom( int idx );
uint8_t *get_chr_rom( int idx );
uint8_t *get_ppu_reg( int idx );

struct mem_map_t *find_map( addr_t addr )
{
    for ( int i = nr_mapper - 1; i >= 0; i-- )
    {
        if ( mapper[ i ].nes_begin > addr )
            continue;

        if ( addr >= mapper[ i ].nes_begin + mapper[ i ].map_size )
        {
            printf( "memory mapping %04x out of bound/not implemented.\n", addr );
            assert( 0 );
        }
        return &mapper[ i ];
    }
    printf( "memory mapping %04x out of bound/not implemented.\n", addr );
    return NULL;
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

    NEW_MAP( 4, "ppu_registers", 0x2000, 0x0008, (uint8_t *) &ppu_reg );
    NEW_MAP( 5, "apu_io_registers", 0x4000, 0x0018, apu_io_reg );

    // TODO: other mapper?
    // The cartridge space at $4020-$FFFF can be used by cartridges for any purpose,
    // such as ROM, RAM, and registers. Many common mappers place ROM and save/work RAM in these locations:
    //      $6000–$7FFF: Battery-backed save or work RAM
    //      $8000–$FFFF: ROM and mapper registers (see MMC1 and UxROM for examples) (from:  nesdev.wiki)
    NEW_MAP( 6, "sram", 0x6000, 0x2000, sram );
    nr_mapper = 7;

    switch ( info->mapper )
    {
    case 0: // https://www.nesdev.org/wiki/NROM
        NEW_MAP( 7, "prg_rom_0", 0x8000, 0x4000, get_prg_rom( 0 ) );
        NEW_MAP( 8, "prg_rom_1", 0xC000, 0x4000, ( info->prg_size == 2 ) ? get_prg_rom( 1 ) : get_prg_rom( 0 ) );
        nr_mapper += 2;
        break;

    default:
        printf( "MAPPER %d NOT IMPLEMENTED\n", info->mapper );
        assert( 0 );
    }

    NMI_VECTOR     = ( vaddr_read( NMI_VECTOR_ADDR + 1 ) << 8 ) + vaddr_read( NMI_VECTOR_ADDR );
    RESET_VECTOR   = ( vaddr_read( RESET_VECTOR_ADDR + 1 ) << 8 ) + vaddr_read( RESET_VECTOR_ADDR );
    IRQ_BRK_VECTOR = ( vaddr_read( IRQ_BRK_VECTOR_ADDR + 1 ) << 8 ) + vaddr_read( IRQ_BRK_VECTOR_ADDR );
    printf( "\nITR Vectors:\n  RESET 0x%04x\n  NMI 0x%04x\n  IRQ/BRK 0x%04x\n", RESET_VECTOR, NMI_VECTOR, IRQ_BRK_VECTOR );

    printf( "Mapper init finished.\n" );
}