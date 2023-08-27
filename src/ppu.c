///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/27
//
//  ppu.c
//
///////////////////////////////////////////////////////////////////////

#include <ppu-reg.h>

struct ppu_reg_t ppu_reg;

inline bool is_ppu_nmi_set() { return ppu_reg.ppuctrl.ppuctrl; }

void init_ppu()
{
}