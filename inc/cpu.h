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

void init_cpu();
void cpu_exec( int n );
void cpu_step();

// https://www.nesdev.org/obelisk-6502-guide/registers.html#N
/**
 * @brief CPU Register Struct
 *
 */
struct cpu_6502_t
{
    addr_t pc;
    uint8_t
        sp,
        accumulator,
        x,
        y;
    union {
        uint8_t ps;
        struct
        {
            uint8_t carry : 1; // D0
            uint8_t zero : 1;
            uint8_t intr_disable : 1;
            uint8_t decimal : 1; // D3

            uint8_t flag_b : 2; // D4
            uint8_t overflow : 1;
            uint8_t negative : 1; // D7
        } flag;
    } status;
};

#define ADDRMODE( mode ) ADDR_MODE_##mode
enum
{
    ADDR_MODE_IMPLICIT,
    ADDR_MODE_ACCUMULATOR,
    ADDR_MODE_IMMEDIATE,
    ADDR_MODE_ZEROPAGE,
    ADDR_MODE_ZEROPAGE_X,
    ADDR_MODE_ZEROPAGE_Y,
    ADDR_MODE_RELATIVE,
    ADDR_MODE_ABSOLUTE,
    ADDR_MODE_ABSOLUTE_X,
    ADDR_MODE_ABSOLUTE_Y,
    ADDR_MODE_INDIRECT,
    ADDR_MODE_INDEXED_INDIRECT,
    ADDR_MODE_INDIRECT_INDEXED,
};

#define CASE( a, ... ) \
    case a:            \
        __VA_ARGS__;   \
        break;
// struct cpu_6502_inst_t
// {
//     char *name;
//     int   addr_mode;
//     void ( *inst_handler )( uint8_t opcode );
//     uint8_t opcode;
// };

#endif // EMULATOR_CPU_H
