///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/21
//
///////////////////////////////////////////////////////////////////////

#include "../inc/rom.h"
#include "nes.h"
#include <cpu.h>
#include <ppu.h>
#include <stdio.h>
#include <stdlib.h>

int main( int argc, char *argv[] )
{
    read_rom_mapper( argc, argv ); // Load ROM and INTERRUPT VECTORS ( nes_rom.c -> memory.c ); Init CPU cpu_memory_mapper
                                   // read_rom() -> read_info() -> init_cpu_mapper()

    init_cpu(); // Init CPU (RESET vector). CPU Power up state...
    init_ppu(); // Init PPU. PPU Power up state...

    nes_mainloop();

    system( "pause" );
    return 0;
}