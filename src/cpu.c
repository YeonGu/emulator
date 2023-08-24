///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/22
//
//  cpu.c
//
///////////////////////////////////////////////////////////////////////

#include <configs.h>
#include <cpu.h>
extern struct cpu_6502_t cpu;
extern addr_t            RESET_VECTOR, NMI_VECTOR, IRQ_BRK_VECTOR;

void cpu_exec_once();

/**
 * CPU Power on
 * */
void init_cpu()
{
    printf( "CPU init...\n" );

    // FIXME: only for TEST!
    cpu.pc        = 0xC000;
    cpu.status.ps = 0x24;
    cpu.sp        = 0xFD;
}

void cpu_exec( int n )
{
    FILE *difftest_file = NULL;
#ifdef CONFIG_DIFFTEST
    difftest_file = fopen( "E:\\0 SEU\\2023\\TiNES\\emulator\\difftest\\cputestlog.txt", "r" );
    assert( difftest_file );
    printf( "Difftest file loaded.\n" );
#endif
    while ( n-- )
    {
        cpu_exec_once( difftest_file );
    }
}
