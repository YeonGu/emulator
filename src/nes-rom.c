///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/21
//
///////////////////////////////////////////////////////////////////////

#include "rom.h"
#include <stdio.h>

char* rom_file;

int read_nes_rom(int argc, char** argv)
{
    if(argc == 1)
        printf("No rom target is given. Use the default mario rom.\n");

    
}