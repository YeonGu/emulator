///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/27
//
//  ppu.c
//
///////////////////////////////////////////////////////////////////////

#include "ppu.h"
#include "memory.h"
#include "ppu-memory.h"
#include "ppu-reg.h"

struct ppu_reg_t      ppu_reg;
struct cpu_mem_map_t *find_cpu_map( addr_t addr );

void ppu_reg_write_handler( addr_t idx, uint8_t data );
void ppu_reg_read_handler( addr_t idx );
void oam_dma_write_handler( addr_t offset, uint8_t data );

void init_ppu()
{
    assert( find_cpu_map( CONFIG_PPU_REG_BASE ) && find_cpu_map( CONFIG_OAM_DMA_BASE ) );
    find_cpu_map( CONFIG_PPU_REG_BASE )->mem_write_handler = ppu_reg_write_handler;
    find_cpu_map( CONFIG_PPU_REG_BASE )->mem_read_handler  = ppu_reg_read_handler;
    find_cpu_map( CONFIG_OAM_DMA_BASE )->mem_write_handler = oam_dma_write_handler;

    init_ppu_memory_map();
}

// https://www.nesdev.org/wiki/PPU_registers
static addr_t vram_addr;
static bool   ppuaddr_w = false;                                // true: write PPUADDR NOT complete.    | false: write PPUADDR complete.
void          ppu_reg_write_handler( addr_t idx, uint8_t data ) // after write
{
    printf( "PPU register write at reg 0x%02x, $%02x\n", idx, data );
    switch ( idx )
    {
    case 0x4: // OAM_DATA. https://www.nesdev.org/wiki/PPU_registers#OAMDATA
              //        As such, you must upload anything to OAM that you intend to within
              //        the first 20 scanlines after the 2C07 signals vertical blanking.
              // Do not write directly to this register in most cases.
              //  Because changes to OAM should normally be made only during vblank,
              //  writing through OAMDATA is only effective for partial updates (it is too slow),
              //  and as described above, partial writes cause corruption. Most games will use the DMA feature through OAMDMA instead.
        // TODO: diable the interface (walled) in 'most cases'
        // TODO: implement the Register...
        printf( "OAM DATA not implemented...\n" );
        break;
    case 0x6: // PPU_ADDR.  >> write x2 (H,L)
        vram_addr <<= 8;
        vram_addr |= ppu_reg.ppuaddr;
        ppuaddr_w = !ppuaddr_w;
        if ( !ppuaddr_w ) vram_addr %= 0x4000;
        break;

    case 0x7: // PPUDATA.   The VRAM address increse is in $2000 bit 2:I (0: add 1, going across; 1: add 32, going down)
              // Write to vram_addr...
        ppu_addr_write( vram_addr, ppu_reg.ppudata );
        vram_addr += ( ppu_reg.ppuctrl.flag.I ) ? 32 : 1;
        break;
    default:
        break;
    }
}
void ppu_reg_read_handler( addr_t idx )
{
    printf( "PPU register read at reg 0x%02x.\n", idx );
    switch ( idx )
    {
    case 0x7: // PPUDATA.   The VRAM address increse is in $2000 bit 2:I (0: add 1, going across; 1: add 32, going down)
        if ( ppuaddr_w )
        {
            printf( "ERROR: The PPU ADDR WRITE IS NOT COMPLETE!!\n" );
            assert( 0 );
        }
        ppu_reg.ppudata = ppu_addr_read( vram_addr );
        vram_addr += ( ppu_reg.ppuctrl.flag.I ) ? 32 : 1;
        break;

    default:
        break;
    }
}

void oam_dma_write_handler( addr_t offset, uint8_t data )
{
}
