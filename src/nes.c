///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/27
//
//  nes.c
//
///////////////////////////////////////////////////////////////////////

#include <cpu.h>
#include <ppu-reg.h>
#include <stdio.h>
void cpu_call_interrupt();

void nes_mainloop()
{
    printf( "Entered NES mainloop.\n" );
    cpu_exec( 15000 );

    set_ppu_nmi( true );
    cpu_call_interrupt();
    cpu_exec( 300 );
    set_ppu_nmi( false );

    printf( "\n\nCYCLE 1....\n\n" );

    int i = 100;
    while ( i-- )
    {
        cpu_exec( 3000 );
        set_ppu_nmi( true );
        cpu_call_interrupt();
        cpu_exec( 300 );
        set_ppu_nmi( false );
    }

    printf( "Exited NES mainloop.\n" );
}
