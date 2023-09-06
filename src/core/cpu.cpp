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
#include <cstdio>
#include <cstdlib>
extern struct cpu_6502_t cpu;
extern addr_t            RESET_VECTOR, NMI_VECTOR, IRQ_BRK_VECTOR;
extern int64_t           nr_cycles;

// void cpu_exec_once( FILE *file );

/**
 * CPU Power on https://www.nesdev.org/wiki/CPU_power_up_state
 * */

void cpu_opcode_register();

FILE           *log_file = nullptr;
extern uint32_t nr_insts_exec;

void init_cpu()
{
    printf( "CPU init...\n" );

    cpu.pc = RESET_VECTOR;
    // FIXME: only for TEST!
    cpu.status.ps = 0x24;
    cpu.sp        = 0xFD;
    nr_insts_exec = 8;

    cpu_opcode_register();
#ifdef CONFIG_LOG_OUTPUT
    log_file = std::fopen( "6502-log.txt", "wb+" );
#endif // CONFIG_LOG_OUTPUT
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
    while ( cycles-- )
    {
        cpu_step();
    }
}
