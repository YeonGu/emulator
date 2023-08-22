///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/23
//
//  cpu_exec.c
//
///////////////////////////////////////////////////////////////////////

#include <cpu.h>

struct cpu_6502_t cpu = {};

void cpu_exec_once();
void cpu_exec();

//////////////////////////////////////////////////////////////////////
void INST_IMPLICIT_HANDLER()
{
}

//////////////////////////////////////////////////////////////////////
#define INSTPAT( inst_name, opcode, ADDR_MODE ) \
    [ opcode ] { inst_name, ADDMODE( ADDR_MODE ), INST_##ADDR_MODE##_HANDLER, opcode }

static struct cpu_6502_inst_t inst[ 256 ] = {
    INSTPAT( "BRK", 0x00, IMPLICIT ),
};
