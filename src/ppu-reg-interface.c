///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/28
//
//  ppu-reg-interface.c
//
///////////////////////////////////////////////////////////////////////

#include <ppu-reg.h>
#include <ppu.h>
#include <stdbool.h>
extern struct ppu_reg_t ppu_reg;
uint8_t                 oamdma;

bool is_ppu_nmi_set() { return ppu_reg.ppustatus.flag.nmi_set; }
void set_ppu_nmi( bool v )
{
    ppu_reg.ppuctrl.flag.V         = true;
    ppu_reg.ppustatus.flag.nmi_set = v;
}