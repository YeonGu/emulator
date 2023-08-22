///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/22
//
//  cpu.h
//
///////////////////////////////////////////////////////////////////////

#ifndef EMULATOR_CPU_H
#define EMULATOR_CPU_H
#include <memory.h>
#include <stdint.h>

// https://www.nesdev.org/obelisk-6502-guide/registers.html#N
struct cpu_6502_t
{
    addr_t pc;
    uint8_t
        sp,
        acu,
        irx,
        iry,
        status;
};

#endif // EMULATOR_CPU_H
