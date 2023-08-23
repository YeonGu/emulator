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

//////////////////////////////////////////////////////////////////////
//  inst handlers of all opcodes
#define HANDLER( mode ) INST_##mode##_HANDLER
#define CASE( a, ... ) \
    case a:            \
        __VA_ARGS__;   \
        break;

// For many 6502 instructions the source and destination
//  of the information to be manipulated is implied directly
//  by the function of the instruction itself and
//  no further operand needs to be specified.
// Operations like 'Clear Carry Flag' (CLC) and 'Return from Subroutine' (RTS) are implicit.
void HANDLER( IMPLICIT )( uint8_t opcode ) {}

// Some instructions have an option to operate **directly upon the accumulator**.
//  The programmer specifies this by using a special operand value, 'A'.
void HANDLER( ACCUMULATOR )( uint8_t opcode ) {}

// Immediate addressing allows the programmer to
// **directly specify an 8 bit constant within the instruction.**
void HANDLER( IMMEDIATE )( uint8_t opcode ) {}

// An instruction using zero page addressing mode
//  has only an 8 bit address operand.
//  This limits it to addressing only the first 256 bytes of memory (e.g. $0000 to $00FF)
//  where the most significant byte of the address is always zero.
// In zero page mode only the least significant byte of the address is held
// in the instruction making it shorter by one byte。
void HANDLER( ZEROPAGE )( uint8_t opcode ) {}

// The address to be accessed by an instruction using indexed zero page addressing
//  is calculated by taking the 8 bit zero page address from the instruction
//  and adding the current value of the X register to it.
//  For example if the X register contains $0F and the instruction LDA $80,
//  X is executed then the accumulator will be loaded from $008F (e.g. $80 + $0F => $8F).
// NB:
//    The address calculation wraps around if the sum of the base address and the register exceed $FF.
//    (e.g. $80 + $FF => $7F) and not $017F.
void HANDLER( ZEROPAGE_X )( uint8_t opcode ) {}

// The address to be accessed by an instruction using indexed zero page addressing
//  is calculated by taking the 8 bit zero page address from the instruction and
//  adding the current value of the Y register to it.
//  This mode can only be used with the LDX and STX instructions.
void HANDLER( ZEROPAGE_Y )( uint8_t opcode ) {}

// Relative addressing mode is used by branch instructions (e.g. BEQ, BNE, etc.)
//  which contain a signed 8 bit relative offset (e.g. -128 to +127) which
//  is added to program counter if the condition is true.
//  As **the program counter itself is incremented during instruction execution by two**
//  the effective address range for the target instruction must be with -126 to +129 bytes of the branch.
void HANDLER( RELATIVE )( uint8_t opcode ) {}

// Instructions using absolute addressing contain a full 16 bit address to identify the target location.
void HANDLER( ABSOLUTE )( uint8_t opcode ) {}

// The address to be accessed by an instruction using X register indexed absolute addressing
//  is computed by taking the 16 bit address from the instruction
//  and added the contents of the X register.
//  For example if X contains $92 then an STA $2000,X instruction will store the accumulator at $2092 (e.g. $2000 + $92).
void HANDLER( ABSOLUTE_X )( uint8_t opcode ) {}

// The Y register indexed absolute addressing mode is the same as
//  the previous mode only with the contents of the Y register added to the 16 bit address from the instruction.
void HANDLER( ABSOLUTE_Y )( uint8_t opcode ) {}

// For example if location $0120 contains $FC and location $0121 contains $BA
//  then the instruction JMP ($0120) will cause
//  the next instruction execution to occur at $BAFC (e.g. the contents of $0120 and $0121).
void HANDLER( INDIRECT )( uint8_t opcode ) {}

// Indexed indirect addressing is normally used in conjunction with a table of address held on zero page.
//  The address of the table is taken from the instruction and the X register added to it
//  (with zero page wrap around) to give the location of the least significant byte of the target address.
void HANDLER( INDEXED_INDIRECT )( uint8_t opcode ) {}

// Indirect index addressing is the most common indirection mode used on the 6502.
//  In instruction contains the zero page location of the least significant byte of 16 bit address.
//  The Y register is dynamically added to this value to generated the actual target address for operation.
void HANDLER( INDIRECT_INDEXED )( uint8_t opcode ) {}

//////////////////////////////////////////////////////////////////////
#define INSTPAT( inst_name, opcode, ADDR_MODE ) \
    [ opcode ] { inst_name, ADDMODE( ADDR_MODE ), INST_##ADDR_MODE##_HANDLER, opcode }

static struct cpu_6502_inst_t inst[ 256 ] = {
    INSTPAT( "BRK", 0x00, IMPLICIT ),
};

//////////////////////////////////////////////////////////////////////
void cpu_exec_once()
{
    uint8_t opcode = vaddr_read( cpu.pc );
    inst[ opcode ].inst_handler( opcode );
}
