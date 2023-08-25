///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/21
//
///////////////////////////////////////////////////////////////////////

#include "../inc/rom.h"
#include <cpu.h>
#include <stdio.h>
#include <stdlib.h>

int main( int argc, char *argv[] )
{
    read_nes_rom( argc, argv ); // Load ROM and INTERRUPT VECTORS ( nes_rom.c -> memory.c ); Init mapper
    init_cpu();                 // Init CPU (RESET vector)

    int test_insts = 1200;
    cpu_exec( test_insts );

    printf( "TiNES execution stopped. CPU Tested %d test insts.\n", test_insts );
    system( "pause" );
    return 0;
}