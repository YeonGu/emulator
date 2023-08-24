///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/21
//
///////////////////////////////////////////////////////////////////////

#include "../inc/rom.h"
#include <cpu.h>
#include <stdio.h>

int main( int argc, char *argv[] )
{
    read_nes_rom( argc, argv ); // Load ROM and INTERRUPT VECTORS ( nes_rom.c -> memory.c ); Init mapper
    init_cpu();                 // Init CPU (RESET vector)

    cpu_exec( 5 );

    return 0;
}