///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/21
//
///////////////////////////////////////////////////////////////////////

#ifndef EMULATOR_ROM_H
#define EMULATOR_ROM_H

///////////////////////////////////////////////////////////////////////////////////////////////////////
//  https://www.nesdev.org/wiki/NES_2.0
//
#include <stdint.h>

struct nes_romhdr_t
{
    char ident[ 4 ];

    uint8_t
        prg_size_l,
        chr_size_l,
        flag6,
        flag7,
        flag8,
        rom_size_h,
        flag10,
        chr_ram_size,
        cpu_ppu_timing,
        flag13,
        flag14,
        flag15;
};
struct nes_rom_info_t
{
    uint16_t prg_size, chr_size;
    int      mapper;
    int      nametable_mirror_type;
};

int  read_rom_mapper( int argc, char **argv );
void print_rom_info();

#endif // EMULATOR_ROM_H
