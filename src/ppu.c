///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/27
//
//  ppu.c
//
///////////////////////////////////////////////////////////////////////

#include <memory.h>
#include <ppu-reg.h>
#include <ppu.h>

struct ppu_reg_t      ppu_reg;
struct cpu_mem_map_t *find_map( addr_t addr );

void ppu_reg_write_handler( addr_t offset, uint8_t data );
void oam_dma_write_handler( addr_t offset, uint8_t data );

void init_ppu()
{
    assert( find_map( CONFIG_PPU_REG_BASE ) && find_map( CONFIG_OAM_DMA_BASE ) );
    find_map( CONFIG_PPU_REG_BASE )->mem_write_handler = ppu_reg_write_handler;
    find_map( CONFIG_OAM_DMA_BASE )->mem_write_handler = oam_dma_write_handler;
}

void ppu_reg_write_handler( addr_t offset, uint8_t data )
{
    switch ( offset )
    {
    case 0x4: // OAM_DATA. https://www.nesdev.org/wiki/PPU_registers#OAMDATA
              //        In the 2C07, sprite evaluation can never be fully disabled,
              //        and will always start 20 scanlines after the start of vblank[9]
              //        (same as when the prerender scanline would have been on the 2C02).
              //        As such, you must upload anything to OAM that you intend to within
              //        the first 20 scanlines after the 2C07 signals vertical blanking.
              // Do not write directly to this register in most cases.
              //  Because changes to OAM should normally be made only during vblank,
              //  writing through OAMDATA is only effective for partial updates (it is too slow),
              //  and as described above, partial writes cause corruption. Most games will use the DMA feature through OAMDMA instead.
        // TODO: diable the interface (walled) in 'most cases'

    default:
        break;
    }
}
void oam_dma_write_handler( addr_t offset, uint8_t data )
{
}
