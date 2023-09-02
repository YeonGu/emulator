///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/21
//
///////////////////////////////////////////////////////////////////////

#include "../inc/rom.h"
#include "nes.h"
#include <cpu.h>
#include <cstdio>
#include <cstdlib>

void init_memory();

int main( int argc, char *argv[] )
{
    init_memory();

    read_rom_mapper( argc, argv ); // Load ROM and INTERRUPT VECTORS ( nes_rom.c -> memory.c ); Init CPU cpu_memory_mapper
                                   // read_rom() -> read_info() -> init_cpu_mapper()

    init_cpu(); // Init CPU (RESET vector). CPU Power up state...
                //    init_ppu(); // Init PPU. PPU Power up state...

    nes_mainloop();

    system( "pause" );
    return 0;
}