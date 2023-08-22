///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/22
//
//  cpu.c
//
///////////////////////////////////////////////////////////////////////

#include <cpu.h>

static struct cpu_6502_t cpu = {};
extern addr_t RESET_VECTOR, NMI_VECTOR, IRQ_BRK_VECTOR;

void init_cpu(){

}
