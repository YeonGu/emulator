//
// Created by 顾雨杭 on 2023/8/27.
//

#ifndef EMULATOR_PPU_REG_H
#define EMULATOR_PPU_REG_H

#include <stdbool.h>
#include <stdint.h>

#define CONFIG_PPU_REG_BASE 0x2000 // https://www.nesdev.org/wiki/CPU_memory_map
#define CONFIG_OAM_DMA_BASE 0x4014

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

    uint8_t ppumask;
    union {
        uint8_t ppustatus;
        struct flags_t
        {
            FLAG( status, 5 );
            FLAG( sprite_overflow, 1 );
            FLAG( sprite0_hit, 1 );
            FLAG( nmi_set, 1 );
        } flag;
    } ppustatus;
    uint8_t oamaddr,
        oamdata,
        ppuscroll,
        ppuaddr,
        ppudata,
        oamdma;
}; // Mapped to ppu_reg in cpu_memory_mapper

bool is_ppu_nmi_set();
bool is_ppu_nmi_enable();

void set_ppu_nmi( bool v );
void set_ppu_nmi_enable( bool v );

#endif // EMULATOR_PPU_REG_H
