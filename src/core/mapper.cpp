///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/27
//
//  cpu_memory_mapper.c
//
///////////////////////////////////////////////////////////////////////

#include "memory.h"
#include "mmu.h"

struct cpu_mem_map_t cpu_memory_mapper[ 16 ];
int                  nr_cpu_mapper;

addr_t RESET_VECTOR, NMI_VECTOR, IRQ_BRK_VECTOR;

static uint8_t internal_ram[ 0x0800 ]; // $0x0800 2KiB internal RAM
static uint8_t apu_io_reg[ 0x0014 ];   // FIXME: not implemented. move to APU module...
static uint8_t apu_snd_chn;
static uint8_t sram[ 0x2000 ];;

/**
 * https://www.nesdev.org/wiki/CPU_memory_map
 * */
// ROM
uint8_t *get_prg_rom( int idx ); // Implemented in NES-ROM, where the rom is read.
uint8_t *get_chr_rom( int idx );
uint8_t *get_ppu_reg( int idx );
// PPU
extern struct ppu_reg_t ppu_reg; // $2000 PPU reg, PPU
extern uint8_t          oamdma;  // $4014 OAM DMA, PPU

#define NEW_MAP( name_, addr_, size_, p ) \
    get_mmu()->mmap(addr_,p,size_)


void init_mapper( struct nes_rom_info_t *info )
{
    //////////////////////////////////////////////////////////////////////
    // Internal Mapper
    for ( int i = 0; i < 4; ++i )
    {
        NEW_MAP( "internal_ram", 0x0800 * i, 0x800, internal_ram );
    }

    // TODO: Mirror Space?
    //    NEW_MAP( "ppu_registers", 0x2000, 0x0008, (uint8_t *) &ppu_reg );
    NEW_MAP( "apu_registers", 0x4000, 0x0014, apu_io_reg );
    //    NEW_MAP( "oamdma", 0x4014, 1, &oamdma );
    NEW_MAP( "snd_chn", 0x4015, 1, &apu_snd_chn );
    //NEW_MAP( "joystick", 0x4016, 2, joystick );

    //////////////////////////////////////////////////////////////////////
    // TODO: other cpu_memory_mapper?
    // The cartridge space at $4020-$FFFF can be used by cartridges for any purpose,
    // such as ROM, RAM, and registers. Many common mappers place ROM and save/work RAM in these locations:
    //      $6000–$7FFF: Battery-backed save or work RAM
    //      $8000–$FFFF: ROM and cpu_memory_mapper registers (see MMC1 and UxROM for examples) (from:  nesdev.wiki)
    NEW_MAP( "sram", 0x6000, 0x2000, sram );
    switch ( info->mapper )
    {
    case 0: // NROM mapping. https://www.nesdev.org/wiki/NROM
        NEW_MAP( "prg_rom_0", 0x8000, 0x4000, get_prg_rom( 0 ) );
        NEW_MAP( "prg_rom_1", 0xC000, 0x4000, ( info->prg_size == 2 ) ? get_prg_rom( 1 ) : get_prg_rom( 0 ) );
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