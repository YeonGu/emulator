///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/21
//
///////////////////////////////////////////////////////////////////////
#define SDL_MAIN_HANDLED
#include "../inc/rom.h"
#include "input_manager.h"
#include "mmu.h"
#include "nes.h"
#include <cpu.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>

void  init_memory();
char *rom_path = nullptr;

int main( int argc, char *argv[] )
{
    if ( argc >= 2 )
        rom_path = argv[ 1 ];

    init_input_manager();

    read_rom_mapper( argc, argv ); // Load ROM and INTERRUPT VECTORS ( nes_rom.c -> memory.c ); Init CPU cpu_memory_mapper
                                   // read_rom() -> read_info() -> init_cpu_mapper()

    init_memory();

    init_cpu(); // Init CPU (RESET vector). CPU Power up state...

    nes_mainloop();

    system( "pause" );
    return 0;
}