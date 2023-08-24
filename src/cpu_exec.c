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
//  of the information to be manipulated is IMPLICIT directly
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

static struct cpu_6502_inst_t inst[ 256 ] = {};

//////////////////////////////////////////////////////////////////////
void cpu_decode_exec( uint8_t opcode );
void cpu_exec_once()
{
    uint8_t opcode = vaddr_read( cpu.pc );
    //    inst[ opcode ].inst_handler( opcode );

    cpu_decode_exec( opcode );
}

//////////////////////////////////////////////////////////////////////
#define INSTPAT( inst_name, code, ADDR_MODE )                                                                        \
    case code:                                                                                                       \
        switch ( ADDRMODE( ADDR_MODE ) )                                                                             \
        {                                                                                                            \
            CASE( ADDRMODE( IMPLICIT ), imm = 0, \
                  cpu.pc += 1 )                                                                    \
            CASE( ADDRMODE( ACCUMULATOR ), imm = 0, \
                  cpu.pc += 1 )                                                                 \
            CASE( ADDRMODE( IMMEDIATE ), imm = vaddr_read( cpu.pc + 1 ), \
                  cpu.pc += 2 )                                            \
            CASE( ADDRMODE( ZEROPAGE ),                                                                              \
                  zeropage_addr = vaddr_read( cpu.pc + 1 ), \
                  cpu.pc += 2 )                                                         \
            CASE( ADDRMODE( ZEROPAGE_X ),                                                                            \
                  zeropage_addr = ( vaddr_read( cpu.pc + 1 ) + cpu.irx ) & 0x00FF, \
                  cpu.pc += 2 )                                  \
                                                                                                                     \
            CASE( ADDRMODE( ZEROPAGE_Y ),                                                                            \
                  zeropage_addr = ( vaddr_read( cpu.pc + 1 ) + cpu.iry ) & 0x00FF, \
                  cpu.pc += 2 )                                  \
                                                                                                                     \
            CASE( ADDRMODE( RELATIVE ),                                                                              \
                  relative_addr = vaddr_read( cpu.pc + 1 ), \
                  cpu.pc += 2 )                                                         \
                                                                                                                     \
            CASE( ADDRMODE( ABSOLUTE ),                                                                              \
                  absolute_addr = vaddr_read( cpu.pc + 1 ) + ( vaddr_read( cpu.pc + 2 ) << 8 ), \
                  cpu.pc += 3 )                     \
                                                                                                                     \
            CASE( ADDRMODE( ABSOLUTE_X ),                                                                            \
                  absolute_addr = vaddr_read( cpu.pc + 1 ) + cpu.irx + ( vaddr_read( cpu.pc + 2 ) << 8 ), \
                  cpu.pc += 3 )           \
                                                                                                                     \
            CASE( ADDRMODE( ABSOLUTE_Y ),                                                                            \
                  absolute_addr = vaddr_read( cpu.pc + 1 ) + cpu.iry + ( vaddr_read( cpu.pc + 2 ) << 8 ), \
                  cpu.pc += 3 )           \
                                                                                                                     \
            CASE( ADDRMODE( INDIRECT ),  \
                  indirect_tmp = vaddr_read( cpu.pc + 1 ) + ( vaddr_read( cpu.pc + 2 ) << 8 ), \
                  indirect_addr = vaddr_read( indirect_tmp ) + ( vaddr_read( indirect_tmp + 1 ) << 8 ), \
                  cpu.pc += 3 )             \
                                                                                                                     \
            CASE( ADDRMODE( INDEXED_INDIRECT ),                                                                      \
                  indirect_tmp  = ( vaddr_read( cpu.pc + 1 ) + cpu.irx ) & 0x00FF,                                   \
                  indirect_addr = vaddr_read( indirect_tmp ) + ( vaddr_read( indirect_tmp + 1 ) << 8 ), \
                  cpu.pc += 2 )             \
                                                                                                                     \
            CASE( ADDRMODE( INDIRECT_INDEXED ),                                                                      \
                  zeropage_addr = vaddr_read( cpu.pc + 1 ),                                                          \
                  indirect_addr = vaddr_read( zeropage_addr ) + cpu.iry + ( vaddr_read( zeropage_addr + 1 ) << 8 ), \
                  cpu.pc += 2) \
        default:                                                                                                     \
            printf( "Unknown ADDR MODE for 0x%02x\n", code );                                                        \
            assert( 0 );                                                                                             \
        }                                                                                                            \
        ##__VA_ARGS__; \
        break;

//    [ opcode ] { inst_name, ADDRMODE( ADDR_MODE ), INST_##ADDR_MODE##_HANDLER, opcode }
void cpu_decode_exec( uint8_t opcode )
{
    uint8_t imm;
    int8_t  imm_signed;
    addr_t  zeropage_addr;
    addr_t  relative_addr;
    addr_t  absolute_addr;
    addr_t  indirect_tmp;
    addr_t  indirect_addr;

    uint8_t ans;

    #define __NOP imm = 0
    #define __OVERFLOW_ADD2_8( a, b ) ((uint16_t) a + (uint16_t)b)>>8
    #define __OVERFLOW_ADD3_8( a, b, c ) ((uint16_t) a + (uint16_t)b + (uint16_t)c)>>8

    switch ( opcode )
    {
            // TODO: BRK Interrupt
        INSTPAT( "BRK", 0x00, IMPLICIT );

        // ADC - Add with Carry
        INSTPAT( "ADC", 0x69, IMMEDIATE, 
            ans = cpu.accumulator + imm + cpu.status.flags.carry );
        INSTPAT( "ADC", 0X65, ZEROPAGE );
        INSTPAT( "ADC", 0x75, ZEROPAGE_X );
        INSTPAT( "ADC", 0x6D, ABSOLUTE );
        INSTPAT( "ADC", 0x7D, ABSOLUTE_X );
        INSTPAT( "ADC", 0x79, ABSOLUTE_Y );
        INSTPAT( "ADC", 0x61, INDEXED_INDIRECT );
        INSTPAT( "ADC", 0x71, INDIRECT_INDEXED );

        // AND -
        INSTPAT( "AND", 0x29, IMMEDIATE );
        INSTPAT( "AND", 0x25, ZEROPAGE );
        INSTPAT( "AND", 0x35, ZEROPAGE_X );
        INSTPAT( "AND", 0x2D, ABSOLUTE );
        INSTPAT( "AND", 0x3D, ABSOLUTE_X );
        INSTPAT( "AND", 0x39, ABSOLUTE_Y );
        INSTPAT( "AND", 0x21, INDEXED_INDIRECT );
        INSTPAT( "AND", 0x31, INDIRECT_INDEXED );

        // ASL - Arithmetic Shift Left
        INSTPAT( "ASL", 0x0A, ACCUMULATOR );
        INSTPAT( "ASL", 0x06, ZEROPAGE );
        INSTPAT( "ASL", 0x16, ZEROPAGE_X );
        INSTPAT( "ASL", 0x0E, ABSOLUTE );
        INSTPAT( "ASL", 0x1E, ABSOLUTE_X );

        // BCC - Branch if Carry Clear
        INSTPAT( "BCC", 0x90, RELATIVE );
        // BCS - Branch if Carry Set
        INSTPAT( "BCS", 0xB0, RELATIVE );
        // BEQ - Branch if Equal
        INSTPAT( "BEQ", 0XF0, RELATIVE );
        // BMI - Branch if Minus
        INSTPAT( "BMI", 0x30, RELATIVE );
        // BNE - Branch if Not Equal
        INSTPAT( "BNE", 0xD0, RELATIVE );
        // BPL - Branch if Positive
        INSTPAT( "BPL", 0x10, RELATIVE );

        // BVC - Branch if Overflow Clear
        INSTPAT( "BVC", 0x50, RELATIVE );

        // BVS - Branch if Overflow Set
        INSTPAT( "BVS", 0x70, RELATIVE );
        // CLC - Clear Carry Flag
        INSTPAT( "CLC", 0x18, IMPLICIT );
        // CLD - Clear Decimal Mode
        INSTPAT( "CLD", 0xD8, IMPLICIT );
        // CLI - Clear Interrupt Disable
        INSTPAT( "CLI", 0x58, IMPLICIT );
        // CLV - Clear Overflow Flag
        INSTPAT( "CLV", 0xB8, IMPLICIT );

        // CMP - Compare
        INSTPAT( "CMP", 0xC9, IMMEDIATE );
        INSTPAT( "CMP", 0xC5, ZEROPAGE );
        INSTPAT( "CMP", 0xD5, ZEROPAGE_X );
        INSTPAT( "CMP", 0xCD, ABSOLUTE );
        INSTPAT( "CMP", 0xDD, ABSOLUTE_X );
        INSTPAT( "CMP", 0xD9, ABSOLUTE_Y );
        INSTPAT( "CMP", 0xC1, INDEXED_INDIRECT );
        INSTPAT( "CMP", 0xD1, INDIRECT_INDEXED );

        // CPX - Compare X Register
        INSTPAT( "CPX", 0xE0, IMMEDIATE );
        INSTPAT( "CPX", 0xE4, ZEROPAGE );
        INSTPAT( "CPX", 0xEC, ABSOLUTE );

        // CPY - Compare Y Register
        INSTPAT( "CPY", 0xC0, IMMEDIATE );
        INSTPAT( "CPY", 0xC4, ZEROPAGE );
        INSTPAT( "CPY", 0xCC, ABSOLUTE );

        // DEC - Decrement Memory
        INSTPAT( "DEC", 0xC6, ZEROPAGE );
        INSTPAT( "DEC", 0xD6, ZEROPAGE_X );
        INSTPAT( "DEC", 0xCE, ABSOLUTE );
        INSTPAT( "DEC", 0xDE, ABSOLUTE_X );

        // DEX - Decrement X Register
        INSTPAT( "DEX", 0xCA, IMPLICIT );

        // DEY - Decrement Y Register
        INSTPAT( "DEY", 0x88, IMPLICIT );

        // EOR - Exclusive OR
        INSTPAT( "EOR", 0x49, IMMEDIATE );
        INSTPAT( "EOR", 0x45, ZEROPAGE );
        INSTPAT( "EOR", 0x55, ZEROPAGE_X );
        INSTPAT( "EOR", 0x4D, ABSOLUTE );
        INSTPAT( "EOR", 0x5D, ABSOLUTE_X );
        INSTPAT( "EOR", 0x59, ABSOLUTE_Y );
        INSTPAT( "EOR", 0x41, INDEXED_INDIRECT );
        INSTPAT( "EOR", 0x51, INDIRECT_INDEXED );

        // INC - Increment Memory
        INSTPAT( "INC", 0xE6, ZEROPAGE );
        INSTPAT( "INC", 0xF6, ZEROPAGE_X );
        INSTPAT( "INC", 0xEE, ABSOLUTE );
        INSTPAT( "INC", 0xFE, ABSOLUTE_X );

        // INX - Increment X Register
        INSTPAT( "INX", 0xE8, IMPLICIT );

        // INY - Increment Y Register
        INSTPAT( "INY", 0xC8, IMPLICIT );

        // JMP - Jump
        INSTPAT( "JMP", 0x4C, ABSOLUTE );
        INSTPAT( "JMP", 0x6C, INDIRECT );

        // JSR - Jump to Subroutine
        INSTPAT( "JSR", 0X20, ABSOLUTE );

        // LDA - Load Accumulator
        INSTPAT( "LDA", 0xA9, IMMEDIATE );
        INSTPAT( "LDA", 0xA5, ZEROPAGE );
        INSTPAT( "LDA", 0xB5, ZEROPAGE_X );
        INSTPAT( "LDA", 0xAD, ABSOLUTE );
        INSTPAT( "LDA", 0xBD, ABSOLUTE_X );
        INSTPAT( "LDA", 0xB9, ABSOLUTE_Y );
        INSTPAT( "LDA", 0xA1, INDEXED_INDIRECT );
        INSTPAT( "LDA", 0xB1, INDIRECT_INDEXED );

        // LDX - Load X Register
        INSTPAT( "LDX", 0xA2, IMMEDIATE );
        INSTPAT( "LDX", 0xA6, ZEROPAGE );
        INSTPAT( "LDX", 0xB6, ZEROPAGE_Y );
        INSTPAT( "LDX", 0xAE, ABSOLUTE );
        INSTPAT( "LDX", 0xBE, ABSOLUTE_Y );

        // LDY - Load Y Register
        INSTPAT( "LDY", 0xA0, IMMEDIATE );
        INSTPAT( "LDY", 0xA4, ZEROPAGE );
        INSTPAT( "LDY", 0xB4, ZEROPAGE_X );
        INSTPAT( "LDY", 0xAC, ABSOLUTE );
        INSTPAT( "LDY", 0xBC, ABSOLUTE_X );

        // LSR - Logical Shift Right
        INSTPAT( "LSR", 0x4A, ACCUMULATOR );
        INSTPAT( "LSR", 0x46, ZEROPAGE );
        INSTPAT( "LSR", 0x56, ZEROPAGE_X );
        INSTPAT( "LSR", 0x4E, ABSOLUTE );
        INSTPAT( "LSR", 0x5E, ABSOLUTE_X );

        // NOP - No Operation
        INSTPAT( "NOP", 0xEA, IMPLICIT );

        // ORA - Logical Inclusive OR
        INSTPAT( "ORA", 0x09, IMMEDIATE );
        INSTPAT( "ORA", 0x05, ZEROPAGE );
        INSTPAT( "ORA", 0x15, ZEROPAGE_X );
        INSTPAT( "ORA", 0x0D, ABSOLUTE );
        INSTPAT( "ORA", 0x1D, ABSOLUTE_X );
        INSTPAT( "ORA", 0x19, ABSOLUTE_Y );
        INSTPAT( "ORA", 0x01, INDEXED_INDIRECT );
        INSTPAT( "ORA", 0x11, INDIRECT_INDEXED );

        // PHA - Push Accumulator
        INSTPAT( "PHA", 0x48, IMPLICIT );

        // PHP - Push Processor Status
        INSTPAT( "PHP", 0x08, IMPLICIT );

        // PLA - Pull Accumulator
        INSTPAT( "PLA", 0x68, IMPLICIT );

        // PLP - Pull Processor Status
        INSTPAT( "PLP", 0x28, IMPLICIT );

        // ROL - Rotate Left
        INSTPAT( "ROL", 0x2A, ACCUMULATOR );
        INSTPAT( "ROL", 0x26, ZEROPAGE );
        INSTPAT( "ROL", 0x36, ZEROPAGE_X );
        INSTPAT( "ROL", 0x2E, ABSOLUTE );
        INSTPAT( "ROL", 0x3E, ABSOLUTE_X );

        // ROR - Rotate Right
        INSTPAT( "ROR", 0x6A, ACCUMULATOR );
        INSTPAT( "ROR", 0x66, ZEROPAGE );
        INSTPAT( "ROR", 0x76, ZEROPAGE_X );
        INSTPAT( "ROR", 0x6E, ABSOLUTE );
        INSTPAT( "ROR", 0x7E, ABSOLUTE_X );

        // RTI - Return from Interrupt
        INSTPAT( "RTI", 0x40, IMPLICIT );

        // RTS - Return from Subroutine
        INSTPAT( "RTS", 0x60, IMPLICIT );

        // SBC - Subtract with Carry
        INSTPAT( "SBC", 0xE9, IMMEDIATE );
        INSTPAT( "SBC", 0xE5, ZEROPAGE );
        INSTPAT( "SBC", 0xF5, ZEROPAGE_X );
        INSTPAT( "SBC", 0xED, ABSOLUTE );
        INSTPAT( "SBC", 0xFD, ABSOLUTE_X );
        INSTPAT( "SBC", 0xF9, ABSOLUTE_Y );
        INSTPAT( "SBC", 0xE1, INDEXED_INDIRECT );
        INSTPAT( "SBC", 0xF1, INDIRECT_INDEXED );

        // SEC - Set Carry Flag
        INSTPAT( "SEC", 0x38, IMPLICIT );

        // SED - Set Decimal Flag
        INSTPAT( "SED", 0xF8, IMPLICIT );

        // SEI - Set Interrupt Disable
        INSTPAT( "SEI", 0x78, IMPLICIT );

        // STA - Store Accumulator
        INSTPAT( "STA", 0x85, ZEROPAGE );
        INSTPAT( "STA", 0x95, ZEROPAGE_Y );
        INSTPAT( "STA", 0x8D, ABSOLUTE );
        INSTPAT( "STA", 0x9D, ABSOLUTE_X );
        INSTPAT( "STA", 0x99, ABSOLUTE_Y );
        INSTPAT( "STA", 0x81, INDEXED_INDIRECT );
        INSTPAT( "STA", 0x91, INDIRECT_INDEXED );

        // STX - Store X Register
        INSTPAT( "STX", 0x86, ZEROPAGE );
        INSTPAT( "STX", 0x96, ZEROPAGE_Y );
        INSTPAT( "STX", 0x8E, ABSOLUTE );
        // STY - Store Y Register
        INSTPAT( "STY", 0x84, ZEROPAGE );
        INSTPAT( "STY", 0x94, ZEROPAGE_X );
        INSTPAT( "STY", 0x8C, ABSOLUTE );

        // TAX - Transfer Accumulator to X
        INSTPAT( "TAX", 0xAA, IMPLICIT );

        // TAY - Transfer Accumulator to Y
        INSTPAT( "TAY", 0xA8, IMPLICIT );

        // TSX - Transfer Stack Pointer to X
        INSTPAT( "TSX", 0xBA, IMPLICIT );

        // TXA - Transfer X to Accumulator
        INSTPAT( "TXA", 0x8A, IMPLICIT );

        // TXS - Transfer X to Stack Pointer
        INSTPAT( "TXS", 0x9A, IMPLICIT );

        // TYA - Transfer Y to Accumulator
        INSTPAT( "TYA", 0x98, IMPLICIT );

    default:
        printf( "UNKNOWN OPCODE.\n" );
        assert( 0 );
    }
}
