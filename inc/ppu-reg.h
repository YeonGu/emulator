//
// Created by 顾雨杭 on 2023/8/27.
//

#ifndef EMULATOR_PPU_REG_H
#define EMULATOR_PPU_REG_H

#include <stdint.h>

#define FLAG( name_, size_ ) uint8_t name_ : size_
struct ppu_reg_t
{
    union {
        uint8_t ppuctrl;
        struct ppuctrl_flags_t
        {
            FLAG( NN, 2 );
            FLAG( I, 1 );
            FLAG( S, 1 );

            FLAG( B, 1 );
            FLAG( H, 1 );
            FLAG( P, 1 );
            FLAG( V, 1 ); // NMI enable
        } flag;
    } ppuctrl; // $2000

    uint8_t ppumask,
        ppustatus,
        oamaddr,
        oamdata,
        ppuscroll,
        ppuaddr,
        ppudata,
        oamdma;
} ppu_reg; // Mapped to ppu_reg in cpu_mapper

#endif // EMULATOR_PPU_REG_H
