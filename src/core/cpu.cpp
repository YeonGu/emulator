///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/22
//
//  cpu.c
//
///////////////////////////////////////////////////////////////////////

#include "cpu.h"
#include "configs.h"
extern struct cpu_6502_t cpu;
extern addr_t            RESET_VECTOR, NMI_VECTOR, IRQ_BRK_VECTOR;
extern int64_t           nr_cycles;

void cpu_exec_once( FILE *file );

/**
 * CPU Power on https://www.nesdev.org/wiki/CPU_power_up_state
 * */

void cpu_opcode_register();

void init_cpu()
{
    printf( "CPU init...\n" );

    // FIXME: only for TEST!
    cpu.status.ps = 0x24;
    cpu.sp        = 0xFD;
    cpu.pc        = RESET_VECTOR;

    cpu_opcode_register();
}

void cpu_exec( int cycles )
{
    FILE    *difftest_file = NULL;
    uint64_t gtime         = nr_cycles;
#ifdef CONFIG_DIFFTEST
    difftest_file = fopen( "E:\\0 SEU\\2023\\TiNES\\emulator\\difftest\\cputestlog.txt", "r" );
    assert( difftest_file );
    printf( "Difftest file loaded.\n" );
#endif
    while ( nr_cycles - gtime <= cycles )
    {
        cpu_exec_once( difftest_file );
    }
}
