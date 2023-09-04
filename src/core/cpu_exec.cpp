///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/23
//
//  cpu_exec.c
//
///////////////////////////////////////////////////////////////////////

#include "configs.h"
#include "cpu.h"
#include "cpu_datas.h"
#include "ppu.h"
#include <functional>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

struct cpu_6502_t cpu = {};
void              cpu_exec_once( FILE *file );
static uint32_t   nr_insts_exec;
int64_t           nr_cycles;
int64_t           current_cycles;

// using byte = uint8_t;
// bool is_ppu_nmi_enable();
// bool is_ppu_nmi_set();
// byte get_vram_inc();
// void set_ppu_nmi_enable( bool v );
// void set_ppu_nmi( bool v );
//////////////////////////////////////////////////////////////////////
// static struct cpu_6502_inst_t inst[ 256 ] = {};

extern addr_t RESET_VECTOR, NMI_VECTOR, IRQ_BRK_VECTOR;
//////////////////////////////////////////////////////////////////////
void cpu_call_nmi();
bool get_nmi_sig();
void cpu_step()
{
    if ( get_nmi_sig() )
    {
        cpu_call_nmi();
        // set_ppu_nmi( false );
    }
    if ( current_cycles++ < nr_cycles )
        return;
    cpu_exec_once( nullptr );
}

void cpu_decode_exec( uint8_t opcode );
void cpu_exec_once( FILE *file )
{

    nr_insts_exec++;
    addr_t  pc     = cpu.pc;
    uint8_t opcode = vaddr_read( cpu.pc );
    //    if ( opcode == 0x40 ) printf( "CPU Return Int\n" );

#ifdef CONFIG_DIFFTEST
    char itrace_log[ 64 ];
    sprintf( itrace_log, "%04X  %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X\n", pc, opcode, cpu.accumulator, cpu.x, cpu.y, cpu.status.ps, cpu.sp );
    printf( "%d| cycles:%d  %s", nr_insts_exec, nr_cycles, itrace_log );
    char itrace_log[ 64 ];
    sprintf( itrace_log, "%04X  %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X\n", pc, opcode, cpu.accumulator, cpu.x, cpu.y, cpu.status.ps, cpu.sp );
    printf( "%d| cycles:%d  %s", nr_insts_exec, nr_cycles, itrace_log );
    char buf[ 64 ];

    // 读取文本，直到碰到新的一行开始
    fgets( buf, 64, file );
    if ( strcmp( itrace_log, buf ) != 0 )
    {
        printf( "\nDIFFTEST ERROR...\n" );
        system( "pause" );
        assert( 0 );
    }
#endif
    cpu_decode_exec( opcode );
}

#define STACKADD( s_ ) ( s_ + 0x100 )
#define STACK_POP_ vaddr_read( STACKADD( ++cpu.sp ) )
#define STACK_PUSH_( data ) vaddr_write( STACKADD( cpu.sp-- ), data )

// Flags in PS
#define CARRY_ cpu.status.flag.carry
#define ZERO_ cpu.status.flag.zero
#define INTR_DIS_ cpu.status.flag.intr_disable
#define DEC_ cpu.status.flag.decimal
#define FLAG_B_ cpu.status.flag.flag_b
#define OVERFLOW_ cpu.status.flag.overflow
#define NEGATIVE_ cpu.status.flag.negative

void cpu_call_nmi()
{
    STACK_PUSH_( cpu.pc >> 8 );
    STACK_PUSH_( cpu.pc );
    nr_cycles += 4;

    FLAG_B_   = 0x10;
    INTR_DIS_ = 1;
    STACK_PUSH_( cpu.status.ps );

    // cpu.status.ps &= 0b11001111;
    // cpu.status.ps |= 0b100000;
    cpu.pc = NMI_VECTOR;
    nr_cycles += 4;
}
void cpu_call_irq()
{
    if ( INTR_DIS_ )
        return;

    STACK_PUSH_( cpu.pc >> 8 );
    STACK_PUSH_( cpu.pc );
    nr_cycles += 4;

    FLAG_B_   = 0b10;
    INTR_DIS_ = 1;
    STACK_PUSH_( cpu.status.ps );

    // cpu.status.ps &= 0b11001111;
    // cpu.status.ps |= 0b100000;
    cpu.pc = IRQ_BRK_VECTOR;
    nr_cycles += 4;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//  Instruction Execution.
//
//////////////////////////////////////////////////////////////////////

#define INSTPAT( inst_name, code, ADDR_MODE, ... )                                                                   \
    opcode_map[ code ] = []() -> void {                                                                              \
        switch ( ADDRMODE( ADDR_MODE ) )                                                                             \
        {                                                                                                            \
            CASE( ADDRMODE( IMPLICIT ), imm = 0,                                                                     \
                  cpu.pc += 1 )                                                                                      \
            CASE( ADDRMODE( ACCUMULATOR ), imm = 0,                                                                  \
                  cpu.pc += 1 )                                                                                      \
            CASE( ADDRMODE( IMMEDIATE ), imm = vaddr_read( cpu.pc + 1 ), M = imm,                                    \
                  cpu.pc += 2 )                                                                                      \
            CASE( ADDRMODE( ZEROPAGE ),                                                                              \
                  zeropage_addr = vaddr_read( cpu.pc + 1 ),                                                          \
                  M             = vaddr_read( zeropage_addr );                                                       \
                  cpu.pc += 2 )                                                                                      \
            CASE( ADDRMODE( ZEROPAGE_X ),                                                                            \
                  zeropage_addr = ( vaddr_read( cpu.pc + 1 ) + cpu.x ) & 0x00FF,                                     \
                  M             = vaddr_read( zeropage_addr );                                                       \
                  cpu.pc += 2 )                                                                                      \
                                                                                                                     \
            CASE( ADDRMODE( ZEROPAGE_Y ),                                                                            \
                  zeropage_addr = ( vaddr_read( cpu.pc + 1 ) + cpu.y ) & 0x00FF,                                     \
                  M             = vaddr_read( zeropage_addr );                                                       \
                  cpu.pc += 2 )                                                                                      \
                                                                                                                     \
            CASE( ADDRMODE( RELATIVE ),                                                                              \
                  relative_addr = vaddr_read( cpu.pc + 1 ),                                                          \
                  cross_page    = ( ( cpu.pc + 2 ) & 0xFF00 ) != ( ( cpu.pc + 2 + relative_addr ) & 0xFF00 ),        \
                  cpu.pc += 2 )                                                                                      \
                                                                                                                     \
            CASE( ADDRMODE( ABSOLUTE ),                                                                              \
                  absolute_addr = vaddr_read( cpu.pc + 1 ) + ( vaddr_read( cpu.pc + 2 ) << 8 ),                      \
                                                                                                                     \
                  cpu.pc += 3 )                                                                                      \
                                                                                                                     \
            CASE( ADDRMODE( ABSOLUTE_X ),                                                                            \
                  absolute_addr = vaddr_read( cpu.pc + 1 ) + ( vaddr_read( cpu.pc + 2 ) << 8 ),                      \
                  cpu.pc += 3,                                                                                       \
                  cross_page = ( absolute_addr & 0xFF00 ) != ( ( absolute_addr + cpu.x ) & 0xFF00 ),                 \
                  absolute_addr += cpu.x,                                                                            \
                  M = vaddr_read( absolute_addr ); )                                                                 \
                                                                                                                     \
            CASE( ADDRMODE( ABSOLUTE_Y ),                                                                            \
                  absolute_addr = vaddr_read( cpu.pc + 1 ) + ( vaddr_read( cpu.pc + 2 ) << 8 ),                      \
                  cpu.pc += 3,                                                                                       \
                  cross_page = ( absolute_addr & 0xFF00 ) != ( ( absolute_addr + cpu.y ) & 0xFF00 ),                 \
                  absolute_addr += cpu.y,                                                                            \
                  M = vaddr_read( absolute_addr ); )                                                                 \
                                                                                                                     \
            CASE( ADDRMODE( INDIRECT ),                                                                              \
                  indirect_tmp  = vaddr_read( cpu.pc + 1 ) + ( vaddr_read( cpu.pc + 2 ) << 8 ),                      \
                  indirect_addr = vaddr_read( indirect_tmp );                                                        \
                  indirect_tmp  = ( ( indirect_tmp + 1 ) & 0x00FF ) | ( indirect_tmp & 0xFF00 ),                     \
                  indirect_addr += ( vaddr_read( indirect_tmp ) << 8 ),                                              \
                  M = vaddr_read( indirect_addr );                                                                   \
                  cpu.pc += 3 )                                                                                      \
                                                                                                                     \
            CASE( ADDRMODE( INDEXED_INDIRECT ),                                                                      \
                  indirect_tmp  = ( vaddr_read( cpu.pc + 1 ) + cpu.x ) & 0x00FF,                                     \
                  indirect_addr = vaddr_read( indirect_tmp ) + ( vaddr_read( ( indirect_tmp + 1 ) & 0x00FF ) << 8 ), \
                  M             = vaddr_read( indirect_addr );                                                       \
                  cpu.pc += 2 )                                                                                      \
                                                                                                                     \
            CASE( ADDRMODE( INDIRECT_INDEXED ),                                                                      \
                  zeropage_addr = vaddr_read( cpu.pc + 1 ),                                                          \
                  indirect_addr = vaddr_read( zeropage_addr ) + ( vaddr_read( ( zeropage_addr + 1 ) & 0xFF ) << 8 ), \
                  cross_page    = ( indirect_addr & 0xFF00 ) != ( ( indirect_addr + cpu.y ) & 0xFF00 ),              \
                  indirect_addr += cpu.y,                                                                            \
                  M = vaddr_read( indirect_addr );                                                                   \
                  cpu.pc += 2 )                                                                                      \
        default:                                                                                                     \
            printf( "Unknown ADDR MODE for 0x%02x\n", code );                                                        \
            assert( 0 );                                                                                             \
        }                                                                                                            \
        nr_cycles += inst_base_cycles[ code ];                                                                       \
        __VA_ARGS__;                                                                                                 \
    }

#define CPU_NOP_ imm = 0

#define CARRY_ADD2_8_( a, b ) ( (uint16_t) a + (uint16_t) b ) >> 8
#define CARRY_ADD3_8_( a, b, c ) ( (uint16_t) a + (uint16_t) b + (uint16_t) c ) >> 8
#define OVERFLOW_2_8_( ans, a, b ) ( ( a >> 7 ) == ( b >> 7 ) ) && ( ( a >> 7 ) != ( ans >> 7 ) )
// #define OVERFLOW_3_8_( ans, a, b ) ( ( a >> 7 ) == ( b >> 7 ) ) && ( ( a >> 7 ) != ( ans >> 7 ) )
#define IS_NEGATIVE_8( a ) ( a >> 7 )

#define SET_CARRY_2_( a, b ) CARRY_ = CARRY_ADD2_8_( a, b )
#define SET_CARRY_3_( a, b, c ) CARRY_ = CARRY_ADD3_8_( a, b, c )
#define SET_ZERO_( a ) ZERO_ = ( a == 0 ) ? 1 : 0
#define SET_INTERRUPT_DISABLE_( a ) INTR_DIS_ = a
#define DISABLE_DEC_ DEC_ = 0
#define SET_OVERFLOW_( a, b ) OVERFLOW_ = OVERFLOW_2_8_( a + b, a, b )
#define SET_NEGATIVE_( a ) NEGATIVE_ = ( a >> 7 )

#define SET_ZERONEG_( a ) SET_ZERO_( a ), SET_NEGATIVE_( a )

#define CYC( a ) nr_cycles += a

std::vector<std::function<void( void )>> opcode_map;

addr_t indirect_tmp;

uint8_t imm;
int8_t  imm_signed;
uint8_t tmp;

uint8_t cross_page;

addr_t  zeropage_addr;
int8_t  relative_addr;
addr_t  absolute_addr;
addr_t  indirect_addr;
uint8_t M;

uint8_t ans;

inline void cpu_decode_exec( uint8_t opcode )
{
    opcode_map[ opcode ]();
    imm = 0;
}

void cpu_opcode_register()
{
    opcode_map = std::vector<std::function<void( void )>>( 0x100, []() -> void {
        printf( "UNKNOWN OPCODE.\n" );
        system( "pause" );
        assert( 0 );
    } );
    INSTPAT( "BRK", 0x00, IMPLICIT,
             STACK_PUSH_( cpu.pc >> 8 ), STACK_PUSH_( cpu.pc ),
             INTR_DIS_ = 1, FLAG_B_ = 0b11, STACK_PUSH_( cpu.status.ps ),
             FLAG_B_ = 0b10,
             cpu.pc  = IRQ_BRK_VECTOR, CYC( 1 ) );

// ADC - Add with Carry
#define ADC_( A_, M_, C_ ) ans = cpu.accumulator + ( M_ + CARRY_ ), SET_OVERFLOW_( A_, M_ ), CARRY_ = CARRY_ADD3_8_( A_, M_, C_ ), SET_ZERO_( ans ), SET_NEGATIVE_( ans ), A_ = ans
    INSTPAT( "ADC", 0x69, IMMEDIATE, ADC_( cpu.accumulator, M, CARRY_ ) );
    INSTPAT( "ADC", 0X65, ZEROPAGE, ADC_( cpu.accumulator, M, CARRY_ ) );
    INSTPAT( "ADC", 0x75, ZEROPAGE_X, ADC_( cpu.accumulator, M, CARRY_ ) );
    INSTPAT( "ADC", 0x6D, ABSOLUTE, M = vaddr_read( absolute_addr ), ADC_( cpu.accumulator, M, CARRY_ ) );
    INSTPAT( "ADC", 0x7D, ABSOLUTE_X, ADC_( cpu.accumulator, M, CARRY_ ), CYC( cross_page ) );
    INSTPAT( "ADC", 0x79, ABSOLUTE_Y, ADC_( cpu.accumulator, M, CARRY_ ), CYC( cross_page ) );
    INSTPAT( "ADC", 0x61, INDEXED_INDIRECT, ADC_( cpu.accumulator, M, CARRY_ ) );
    INSTPAT( "ADC", 0x71, INDIRECT_INDEXED, ADC_( cpu.accumulator, M, CARRY_ ), CYC( cross_page ) );

// AND -
#define AND_( M_ ) cpu.accumulator &= M_, SET_ZERONEG_( M_ )
    INSTPAT( "AND", 0x29, IMMEDIATE,
             cpu.accumulator &= imm,
             SET_ZERO_( cpu.accumulator ), SET_NEGATIVE_( cpu.accumulator ) );
    INSTPAT( "AND", 0x25, ZEROPAGE,
             cpu.accumulator &= vaddr_read( zeropage_addr ),
             SET_ZERO_( cpu.accumulator ), SET_NEGATIVE_( cpu.accumulator ) );
    INSTPAT( "AND", 0x35, ZEROPAGE_X,
             cpu.accumulator &= vaddr_read( zeropage_addr ),
             SET_ZERO_( cpu.accumulator ), SET_NEGATIVE_( cpu.accumulator ) );
    INSTPAT( "AND", 0x2D, ABSOLUTE,
             cpu.accumulator &= vaddr_read( absolute_addr ),
             SET_ZERO_( cpu.accumulator ), SET_NEGATIVE_( cpu.accumulator ) );
    INSTPAT( "AND", 0x3D, ABSOLUTE_X,
             cpu.accumulator &= vaddr_read( absolute_addr ),
             SET_ZERO_( cpu.accumulator ), SET_NEGATIVE_( cpu.accumulator ), CYC( cross_page ) );
    INSTPAT( "AND", 0x39, ABSOLUTE_Y,
             cpu.accumulator &= vaddr_read( absolute_addr ),
             SET_ZERO_( cpu.accumulator ), SET_NEGATIVE_( cpu.accumulator ), CYC( cross_page ) );
    INSTPAT( "AND", 0x21, INDEXED_INDIRECT,
             cpu.accumulator &= vaddr_read( indirect_addr ),
             SET_ZERO_( cpu.accumulator ), SET_NEGATIVE_( cpu.accumulator ) );
    INSTPAT( "AND", 0x31, INDIRECT_INDEXED,
             cpu.accumulator &= vaddr_read( indirect_addr ),
             SET_ZERO_( cpu.accumulator ), SET_NEGATIVE_( cpu.accumulator ), CYC( cross_page ) );

// ASL - Arithmetic Shift Left
#define ASL_M_( addr, M_ ) CARRY_ = M_ >> 7, M_ <<= 1, SET_ZERONEG_( M_ ), vaddr_write( addr, M_ )
    INSTPAT( "ASL", 0x0A, ACCUMULATOR,
             CARRY_ = cpu.accumulator >> 7, cpu.accumulator <<= 1, SET_ZERONEG_( cpu.accumulator ) );
    INSTPAT( "ASL", 0x06, ZEROPAGE, ASL_M_( zeropage_addr, M ) );
    INSTPAT( "ASL", 0x16, ZEROPAGE_X, ASL_M_( zeropage_addr, M ) );
    INSTPAT( "ASL", 0x0E, ABSOLUTE, M = vaddr_read( absolute_addr ), ASL_M_( absolute_addr, M ) );
    INSTPAT( "ASL", 0x1E, ABSOLUTE_X, ASL_M_( absolute_addr, M ) );

    // BIT - Bit Test
    INSTPAT( "BIT", 0x24, ZEROPAGE,
             SET_ZERO_( ( vaddr_read( zeropage_addr ) & cpu.accumulator ) ),
             SET_NEGATIVE_( vaddr_read( zeropage_addr ) ), OVERFLOW_ = ( vaddr_read( zeropage_addr ) >> 6 ) & 1 );
    INSTPAT( "BIT", 0x2C, ABSOLUTE,
             SET_ZERO_( ( vaddr_read( absolute_addr ) & cpu.accumulator ) ),
             SET_NEGATIVE_( vaddr_read( absolute_addr ) ), OVERFLOW_ = vaddr_read( absolute_addr ) >> 6 & 1 );

    // BCC - Branch if Carry Clear
    INSTPAT( "BCC", 0x90, RELATIVE,
             cpu.pc += ( CARRY_ ) ? 0 : relative_addr, CYC( ( !CARRY_ ) + ( cross_page & ( !CARRY_ ) ) ) );
    // BCS - Branch if Carry Set
    INSTPAT( "BCS", 0xB0, RELATIVE,
             cpu.pc += ( CARRY_ ) ? relative_addr : 0, CYC( ( CARRY_ ) + ( cross_page & ( CARRY_ ) ) ) );
    // BEQ - Branch if Equal
    INSTPAT( "BEQ", 0XF0, RELATIVE,
             cpu.pc += ( ZERO_ ) ? relative_addr : 0, CYC( ( ZERO_ ) + ( cross_page & ( ZERO_ ) ) ) );
    // BMI - Branch if Minus
    INSTPAT( "BMI", 0x30, RELATIVE,
             cpu.pc += ( NEGATIVE_ ) ? relative_addr : 0, CYC( ( NEGATIVE_ ) + ( cross_page & ( NEGATIVE_ ) ) ) );
    // BNE - Branch if Not Equal
    INSTPAT( "BNE", 0xD0, RELATIVE,
             cpu.pc += ( !ZERO_ ) ? relative_addr : 0, CYC( ( !ZERO_ ) + ( cross_page & ( !ZERO_ ) ) ) );
    // BPL - Branch if Positive
    INSTPAT( "BPL", 0x10, RELATIVE,
             cpu.pc += ( !NEGATIVE_ ) ? relative_addr : 0, CYC( ( !NEGATIVE_ ) + ( cross_page & ( !NEGATIVE_ ) ) ) );
    // BVC - Branch if Overflow Clear
    INSTPAT( "BVC", 0x50, RELATIVE,
             cpu.pc += ( !OVERFLOW_ ) ? ( int16_t ) * (int8_t *) ( &relative_addr ) : 0, CYC( ( !OVERFLOW_ ) + ( cross_page & ( !OVERFLOW_ ) ) ) );
    // BVS - Branch if Overflow Set
    INSTPAT( "BVS", 0x70, RELATIVE,
             cpu.pc += ( OVERFLOW_ ) ? relative_addr : 0, CYC( ( OVERFLOW_ ) + ( cross_page & ( OVERFLOW_ ) ) ) );

    // CLC - Clear Carry Flag
    INSTPAT( "CLC", 0x18, IMPLICIT, CARRY_ = 0 );
    // CLD - Clear Decimal Mode
    INSTPAT( "CLD", 0xD8, IMPLICIT, DEC_ = 0 );
    // CLI - Clear Interrupt Disable
    INSTPAT( "CLI", 0x58, IMPLICIT, SET_INTERRUPT_DISABLE_( 0 ) );
    // CLV - Clear Overflow Flag
    INSTPAT( "CLV", 0xB8, IMPLICIT, OVERFLOW_ = 0 );

// CMP - Compare
#define CMP_( M_ ) CARRY_ = ( cpu.accumulator >= M_ ), SET_ZERO_( cpu.accumulator - M_ ), SET_NEGATIVE_( cpu.accumulator - M_ )
    INSTPAT( "CMP", 0xC9, IMMEDIATE,
             CARRY_ = ( cpu.accumulator >= imm ), SET_ZERO_( cpu.accumulator - imm ), SET_NEGATIVE_( cpu.accumulator - imm ) );
    INSTPAT( "CMP", 0xC5, ZEROPAGE,
             CARRY_ = ( cpu.accumulator >= M ), SET_ZERO_( cpu.accumulator - M ), SET_NEGATIVE_( cpu.accumulator - M ) );
    INSTPAT( "CMP", 0xD5, ZEROPAGE_X,
             CARRY_ = ( cpu.accumulator >= M ), SET_ZERO_( cpu.accumulator - M ), SET_NEGATIVE_( cpu.accumulator - M ) );
    INSTPAT( "CMP", 0xCD, ABSOLUTE, M = vaddr_read( absolute_addr ),
             CARRY_ = ( cpu.accumulator >= M ), SET_ZERO_( cpu.accumulator - M ), SET_NEGATIVE_( cpu.accumulator - M ) );
    INSTPAT( "CMP", 0xDD, ABSOLUTE_X,
             CARRY_ = ( cpu.accumulator >= M ), SET_ZERO_( cpu.accumulator - M ), SET_NEGATIVE_( cpu.accumulator - M ), CYC( cross_page ) );
    INSTPAT( "CMP", 0xD9, ABSOLUTE_Y,
             CARRY_ = ( cpu.accumulator >= M ), SET_ZERO_( cpu.accumulator - M ), SET_NEGATIVE_( cpu.accumulator - M ), CYC( cross_page ) );
    INSTPAT( "CMP", 0xC1, INDEXED_INDIRECT,
             CARRY_ = ( cpu.accumulator >= M ), SET_ZERO_( cpu.accumulator - M ), SET_NEGATIVE_( cpu.accumulator - M ) );
    INSTPAT( "CMP", 0xD1, INDIRECT_INDEXED,
             CARRY_ = ( cpu.accumulator >= M ), SET_ZERO_( cpu.accumulator - M ), SET_NEGATIVE_( cpu.accumulator - M ), CYC( cross_page ) );

    // CPX - Compare X Register
#define CPX_( M_ ) CARRY_ = ( cpu.x >= M_ ), SET_ZERO_( cpu.x - M_ ), SET_NEGATIVE_( ( cpu.x - M_ ) )
    INSTPAT( "CPX", 0xE0, IMMEDIATE, CPX_( M ) );
    INSTPAT( "CPX", 0xE4, ZEROPAGE, CPX_( M ) );
    INSTPAT( "CPX", 0xEC, ABSOLUTE, M = vaddr_read( absolute_addr ), CPX_( M ) );

// CPY - Compare Y Register
#define CPY_( M_ ) CARRY_ = ( cpu.y >= M_ ), SET_ZERO_( cpu.y - M_ ), SET_NEGATIVE_( ( cpu.y - M_ ) )
    INSTPAT( "CPY", 0xC0, IMMEDIATE, CPY_( M ) );
    INSTPAT( "CPY", 0xC4, ZEROPAGE, CPY_( M ) );
    INSTPAT( "CPY", 0xCC, ABSOLUTE, M = vaddr_read( absolute_addr ), CPY_( M ) );

// DEC - Decrement Memory
#define DECRM_( addr, M_ ) ans = --M_, vaddr_write( addr, ans ), SET_ZERO_( ans ), SET_NEGATIVE_( ans )
    INSTPAT( "DEC", 0xC6, ZEROPAGE, DECRM_( zeropage_addr, M ) );
    INSTPAT( "DEC", 0xD6, ZEROPAGE_X, DECRM_( zeropage_addr, M ) );
    INSTPAT( "DEC", 0xCE, ABSOLUTE, M = vaddr_read( absolute_addr ), DECRM_( absolute_addr, M ) );
    INSTPAT( "DEC", 0xDE, ABSOLUTE_X, DECRM_( absolute_addr, M ) );

    // DEX - Decrement X Register
    INSTPAT( "DEX", 0xCA, IMPLICIT, cpu.x--, SET_ZERO_( cpu.x ), SET_NEGATIVE_( cpu.x ) );
    // DEY - Decrement Y Register
    INSTPAT( "DEY", 0x88, IMPLICIT, cpu.y--, SET_ZERO_( cpu.y ), SET_NEGATIVE_( cpu.y ) );

// EOR - Exclusive OR
#define EOR_( A_, M_ ) A_ ^= M_, SET_ZERO_( A_ ), SET_NEGATIVE_( A_ )
    INSTPAT( "EOR", 0x49, IMMEDIATE, EOR_( cpu.accumulator, imm ) );
    INSTPAT( "EOR", 0x45, ZEROPAGE, EOR_( cpu.accumulator, M ) );
    INSTPAT( "EOR", 0x55, ZEROPAGE_X, EOR_( cpu.accumulator, M ) );
    INSTPAT( "EOR", 0x4D, ABSOLUTE, M = vaddr_read( absolute_addr ), EOR_( cpu.accumulator, M ) );
    INSTPAT( "EOR", 0x5D, ABSOLUTE_X, EOR_( cpu.accumulator, M ), CYC( cross_page ) );
    INSTPAT( "EOR", 0x59, ABSOLUTE_Y, EOR_( cpu.accumulator, M ), CYC( cross_page ) );
    INSTPAT( "EOR", 0x41, INDEXED_INDIRECT, EOR_( cpu.accumulator, M ) );
    INSTPAT( "EOR", 0x51, INDIRECT_INDEXED, EOR_( cpu.accumulator, M ), CYC( cross_page ) );

// INC - Increment Memory
#define INC_( addr, M_ ) vaddr_write( addr, ++M_ ), SET_ZERO_( M_ ), SET_NEGATIVE_( ( M_ ) )
    INSTPAT( "INC", 0xE6, ZEROPAGE, INC_( zeropage_addr, M ) );
    INSTPAT( "INC", 0xF6, ZEROPAGE_X, INC_( zeropage_addr, M ) );
    INSTPAT( "INC", 0xEE, ABSOLUTE, M = vaddr_read( absolute_addr ), INC_( absolute_addr, M ) );
    INSTPAT( "INC", 0xFE, ABSOLUTE_X, INC_( absolute_addr, M ) );

    // INX - Increment X Register
    INSTPAT( "INX", 0xE8, IMPLICIT, cpu.x++, SET_ZERO_( cpu.x ), SET_NEGATIVE_( cpu.x ) );
    // INY - Increment Y Register
    INSTPAT( "INY", 0xC8, IMPLICIT, cpu.y++, SET_ZERO_( cpu.y ), SET_NEGATIVE_( cpu.y ) );

    // JMP - Jump
    INSTPAT( "JMP", 0x4C, ABSOLUTE,
             cpu.pc = absolute_addr );
    INSTPAT( "JMP", 0x6C, INDIRECT,
             cpu.pc = indirect_addr );

    // JSR - Jump to Subroutine
    INSTPAT( "JSR", 0X20, ABSOLUTE,
             vaddr_write( STACKADD( cpu.sp-- ), ( cpu.pc - 1 ) >> 8 ), vaddr_write( STACKADD( cpu.sp-- ), ( cpu.pc - 1 ) ), cpu.pc = absolute_addr );

    // LDA - Load Accumulator
#define LDA_( M_ ) cpu.accumulator = M_, SET_ZERO_( cpu.accumulator ), SET_NEGATIVE_( cpu.accumulator )
    INSTPAT( "LDA", 0xA9, IMMEDIATE, LDA_( M ) );
    INSTPAT( "LDA", 0xA5, ZEROPAGE, LDA_( M ) );
    INSTPAT( "LDA", 0xB5, ZEROPAGE_X, LDA_( M ) );
    INSTPAT( "LDA", 0xAD, ABSOLUTE, M = vaddr_read( absolute_addr ), LDA_( M ) );
    INSTPAT( "LDA", 0xBD, ABSOLUTE_X, LDA_( M ), CYC( cross_page ) );
    INSTPAT( "LDA", 0xB9, ABSOLUTE_Y, LDA_( M ), CYC( cross_page ) );
    INSTPAT( "LDA", 0xA1, INDEXED_INDIRECT, LDA_( M ) );
    INSTPAT( "LDA", 0xB1, INDIRECT_INDEXED, LDA_( M ), CYC( cross_page ) );

// LDX - Load X Register
#define LDX_( M_ ) cpu.x = M_, SET_ZERO_( cpu.x ), SET_NEGATIVE_( cpu.x )
    INSTPAT( "LDX", 0xA2, IMMEDIATE, LDX_( M ) );
    INSTPAT( "LDX", 0xA6, ZEROPAGE, LDX_( M ) );
    INSTPAT( "LDX", 0xB6, ZEROPAGE_Y, LDX_( M ) );
    INSTPAT( "LDX", 0xAE, ABSOLUTE, M = vaddr_read( absolute_addr ), LDX_( M ) );
    INSTPAT( "LDX", 0xBE, ABSOLUTE_Y, LDX_( M ), CYC( cross_page ) );

// LDY - Load Y Register
#define LDY_( M_ ) cpu.y = M_, SET_ZERO_( M_ ), SET_NEGATIVE_( M_ )
    INSTPAT( "LDY", 0xA0, IMMEDIATE, LDY_( M ) );
    INSTPAT( "LDY", 0xA4, ZEROPAGE, LDY_( M ) );
    INSTPAT( "LDY", 0xB4, ZEROPAGE_X, LDY_( M ) );
    INSTPAT( "LDY", 0xAC, ABSOLUTE, M = vaddr_read( absolute_addr ), LDY_( M ) );
    INSTPAT( "LDY", 0xBC, ABSOLUTE_X, LDY_( M ), CYC( cross_page ) );

// LSR - Logical Shift Right
#define LSR_M_( addr, M_ ) CARRY_ = M_ & 0x01, vaddr_write( addr, M_ >>= 1 ), SET_ZERO_( M_ ), NEGATIVE_ = 0
    INSTPAT( "LSR", 0x4A, ACCUMULATOR,
             CARRY_ = cpu.accumulator & 0x01, cpu.accumulator >>= 1, SET_ZERO_( cpu.accumulator ), NEGATIVE_ = 0 );
    INSTPAT( "LSR", 0x46, ZEROPAGE, LSR_M_( zeropage_addr, M ) );
    INSTPAT( "LSR", 0x56, ZEROPAGE_X, LSR_M_( zeropage_addr, M ) );
    INSTPAT( "LSR", 0x4E, ABSOLUTE, M = vaddr_read( absolute_addr ), LSR_M_( absolute_addr, M ) );
    INSTPAT( "LSR", 0x5E, ABSOLUTE_X, LSR_M_( absolute_addr, M ) );

    // NOP - No Operation
    INSTPAT( "NOP", 0xEA, IMPLICIT, CPU_NOP_ );

// ORA - Logical Inclusive OR
#define ORA_( _A, _M ) _A |= _M, SET_ZERO_( _A ), SET_NEGATIVE_( _A )
    INSTPAT( "ORA", 0x09, IMMEDIATE, ORA_( cpu.accumulator, imm ) );
    INSTPAT( "ORA", 0x05, ZEROPAGE, ORA_( cpu.accumulator, M ) );
    INSTPAT( "ORA", 0x15, ZEROPAGE_X, ORA_( cpu.accumulator, M ) );
    INSTPAT( "ORA", 0x0D, ABSOLUTE, M = vaddr_read( absolute_addr ), ORA_( cpu.accumulator, M ) );
    INSTPAT( "ORA", 0x1D, ABSOLUTE_X, ORA_( cpu.accumulator, M ), CYC( cross_page ) );
    INSTPAT( "ORA", 0x19, ABSOLUTE_Y, ORA_( cpu.accumulator, M ), CYC( cross_page ) );
    INSTPAT( "ORA", 0x01, INDEXED_INDIRECT, ORA_( cpu.accumulator, M ) );
    INSTPAT( "ORA", 0x11, INDIRECT_INDEXED, ORA_( cpu.accumulator, M ), CYC( cross_page ) );

    // PHA - Push Accumulator
    INSTPAT( "PHA", 0x48, IMPLICIT, STACK_PUSH_( cpu.accumulator ) );
    // PHP - Push Processor Status
    INSTPAT( "PHP", 0x08, IMPLICIT,
             FLAG_B_ = 0b11, STACK_PUSH_( cpu.status.ps ),
             FLAG_B_ = 0 );
    // PLA - Pull Accumulator
    INSTPAT( "PLA", 0x68, IMPLICIT,
             cpu.accumulator = STACK_POP_,
             SET_ZERO_( cpu.accumulator ), SET_NEGATIVE_( cpu.accumulator ) );
    // PLP - Pull Processor Status
    INSTPAT( "PLP", 0x28, IMPLICIT, cpu.status.ps = STACK_POP_ & 0xEF | 0b100000 );

// ROL - Rotate Left
#define ROL_( M_ ) tmp = M_ >> 7, M_ <<= 1, M_ |= CARRY_, CARRY_ = tmp, SET_ZERONEG_( M_ )
    INSTPAT( "ROL", 0x2A, ACCUMULATOR, ROL_( cpu.accumulator ) );
    INSTPAT( "ROL", 0x26, ZEROPAGE, ROL_( M ), vaddr_write( zeropage_addr, M ) );
    INSTPAT( "ROL", 0x36, ZEROPAGE_X, ROL_( M ), vaddr_write( zeropage_addr, M ) );
    INSTPAT( "ROL", 0x2E, ABSOLUTE, M = vaddr_read( absolute_addr ), ROL_( M ), vaddr_write( absolute_addr, M ) );
    INSTPAT( "ROL", 0x3E, ABSOLUTE_X, ROL_( M ), vaddr_write( absolute_addr, M ) );

    // ROR - Rotate Right
#define ROR_( M_ ) tmp = M_ & 0x01, M_ >>= 1, M_ |= ( CARRY_ << 7 ), CARRY_ = tmp, SET_ZERONEG_( M_ )
    INSTPAT( "ROR", 0x6A, ACCUMULATOR, ROR_( cpu.accumulator ) );
    INSTPAT( "ROR", 0x66, ZEROPAGE, ROR_( M ), vaddr_write( zeropage_addr, M ) );
    INSTPAT( "ROR", 0x76, ZEROPAGE_X, ROR_( M ), vaddr_write( zeropage_addr, M ) );
    INSTPAT( "ROR", 0x6E, ABSOLUTE, M = vaddr_read( absolute_addr ), ROR_( M ), vaddr_write( absolute_addr, M ) );
    INSTPAT( "ROR", 0x7E, ABSOLUTE_X, ROR_( M ), vaddr_write( absolute_addr, M ) );

    // RTI - Return from Interrupt
    INSTPAT( "RTI", 0x40, IMPLICIT, cpu.status.ps = STACK_POP_,
             cpu.pc = STACK_POP_, cpu.pc |= ( STACK_POP_ << 8 ),
             cpu.status.ps &= 0xEF, cpu.status.flag.flag_b = 1, CYC( 1 ) ); // Set flagb to 2'b01

    // RTS - Return from Subroutine
    INSTPAT( "RTS", 0x60, IMPLICIT,
             cpu.pc = 1 + vaddr_read( STACKADD( cpu.sp + 1 ) ) + ( (addr_t) vaddr_read( STACKADD( cpu.sp + 2 ) ) << 8 ),
             cpu.sp += 2, CYC( 1 ) );

// SBC - Subtract with Carry
#define SBC_( A, M_ ) ans       = A - M_ - ( 1 - CARRY_ ),                         \
                      OVERFLOW_ = ( ( A ^ M_ ) & 0x80 ) && ( ( A ^ ans ) & 0x80 ), \
                      SET_CARRY_2_( A - M_, CARRY_ - 1 ),                          \
                      CARRY_ = ~CARRY_, SET_ZERO_( ans ),                          \
                      SET_NEGATIVE_( ans ), A = ans
    INSTPAT( "SBC", 0xE9, IMMEDIATE, SBC_( cpu.accumulator, M ) );
    INSTPAT( "SBC", 0xE5, ZEROPAGE, SBC_( cpu.accumulator, M ) );
    INSTPAT( "SBC", 0xF5, ZEROPAGE_X, SBC_( cpu.accumulator, M ) );
    INSTPAT( "SBC", 0xED, ABSOLUTE, M = vaddr_read( absolute_addr ), SBC_( cpu.accumulator, M ) );
    INSTPAT( "SBC", 0xFD, ABSOLUTE_X, SBC_( cpu.accumulator, M ), CYC( cross_page ) );
    INSTPAT( "SBC", 0xF9, ABSOLUTE_Y, SBC_( cpu.accumulator, M ), CYC( cross_page ) );
    INSTPAT( "SBC", 0xE1, INDEXED_INDIRECT, SBC_( cpu.accumulator, M ) );
    INSTPAT( "SBC", 0xF1, INDIRECT_INDEXED, SBC_( cpu.accumulator, M ), CYC( cross_page ) );
    // unofficial...
    INSTPAT( "SBC", 0xEB, IMMEDIATE, SBC_( cpu.accumulator, M ) );

    // SEC - Set Carry Flag
    INSTPAT( "SEC", 0x38, IMPLICIT, CARRY_ = 1 );
    // SED - Set Decimal Flag
    INSTPAT( "SED", 0xF8, IMPLICIT, DEC_ = 1 );
    // SEI - Set Interrupt Disable
    INSTPAT( "SEI", 0x78, IMPLICIT, SET_INTERRUPT_DISABLE_( 1 ) );

// STA - Store Accumulator
#define STA_( addr ) vaddr_write( addr, cpu.accumulator )
    INSTPAT( "STA", 0x85, ZEROPAGE, STA_( zeropage_addr ) );
    INSTPAT( "STA", 0x95, ZEROPAGE_X, STA_( zeropage_addr ) );
    INSTPAT( "STA", 0x8D, ABSOLUTE, STA_( absolute_addr ) );
    INSTPAT( "STA", 0x9D, ABSOLUTE_X, STA_( absolute_addr ) );
    INSTPAT( "STA", 0x99, ABSOLUTE_Y, STA_( absolute_addr ) );
    INSTPAT( "STA", 0x81, INDEXED_INDIRECT, STA_( indirect_addr ) );
    INSTPAT( "STA", 0x91, INDIRECT_INDEXED, STA_( indirect_addr ) );

    // STX - Store X Register
    INSTPAT( "STX", 0x86, ZEROPAGE,
             vaddr_write( zeropage_addr, cpu.x ) );
    INSTPAT( "STX", 0x96, ZEROPAGE_Y,
             vaddr_write( zeropage_addr, cpu.x ) );
    INSTPAT( "STX", 0x8E, ABSOLUTE,
             vaddr_write( absolute_addr, cpu.x ) );
    // STY - Store Y Register
    INSTPAT( "STY", 0x84, ZEROPAGE,
             vaddr_write( zeropage_addr, cpu.y ) );
    INSTPAT( "STY", 0x94, ZEROPAGE_X,
             vaddr_write( zeropage_addr, cpu.y ) );
    INSTPAT( "STY", 0x8C, ABSOLUTE,
             vaddr_write( absolute_addr, cpu.y ) );

    // INSTPAT( "SEC", 0x38, IMPLICIT, CARRY_ = 1 );

#define TRANS_( from_, to_ ) to_ = from_, SET_ZERO_( to_ ), SET_NEGATIVE_( to_ )
    // TAX - Transfer Accumulator to X
    INSTPAT( "TAX", 0xAA, IMPLICIT, TRANS_( cpu.accumulator, cpu.x ) );
    // TAY - Transfer Accumulator to Y
    INSTPAT( "TAY", 0xA8, IMPLICIT, TRANS_( cpu.accumulator, cpu.y ) );
    // TSX - Transfer Stack Pointer to X
    INSTPAT( "TSX", 0xBA, IMPLICIT, TRANS_( cpu.sp, cpu.x ) );
    // TXA - Transfer X to Accumulator
    INSTPAT( "TXA", 0x8A, IMPLICIT, TRANS_( cpu.x, cpu.accumulator ) );
    // TXS - Transfer X to Stack Pointer
    INSTPAT( "TXS", 0x9A, IMPLICIT, cpu.sp = cpu.x );
    // TYA - Transfer Y to Accumulator
    INSTPAT( "TYA", 0x98, IMPLICIT, TRANS_( cpu.y, cpu.accumulator ) );

    // Then there comes **unofficial opcodes**...
    //
    // LAX - Load Accumulator / X
#define LAX_( M_ ) cpu.accumulator = ( cpu.x = M_ ), SET_ZERO_( cpu.accumulator ), SET_NEGATIVE_( cpu.accumulator )
    INSTPAT( "LAX", 0xAB, IMMEDIATE, LAX_( M ) );
    INSTPAT( "LAX", 0xA7, ZEROPAGE, LAX_( M ) );
    INSTPAT( "LAX", 0xB7, ZEROPAGE_Y, LAX_( M ) );
    INSTPAT( "LAX", 0xAF, ABSOLUTE, M = vaddr_read( absolute_addr ), LAX_( M ) );
    INSTPAT( "LAX", 0xBF, ABSOLUTE_Y, LAX_( M ), CYC( cross_page ) );
    INSTPAT( "LAX", 0xA3, INDEXED_INDIRECT, LAX_( M ) );
    INSTPAT( "LAX", 0xB3, INDIRECT_INDEXED, LAX_( M ), CYC( cross_page ) );

    // SAX - Store Accumulator & x
#define SAX_( addr ) vaddr_write( addr, cpu.accumulator &cpu.x )
    INSTPAT( "SAX", 0x87, ZEROPAGE, SAX_( zeropage_addr ) );
    INSTPAT( "SAX", 0x97, ZEROPAGE_Y, SAX_( zeropage_addr ) );
    INSTPAT( "SAX", 0x8F, ABSOLUTE, SAX_( absolute_addr ) );
    INSTPAT( "SAX", 0x83, INDEXED_INDIRECT, SAX_( indirect_addr ) );

#define DCP( addr_, M_ ) DECRM_( addr_, M_ ), CMP_( M_ )
    INSTPAT( "DCP", 0xCF, ABSOLUTE, M = vaddr_read( absolute_addr ), DCP( absolute_addr, M ) );
    INSTPAT( "DCP", 0xDF, ABSOLUTE_X, DCP( absolute_addr, M ) );
    INSTPAT( "DCP", 0xDB, ABSOLUTE_Y, DCP( absolute_addr, M ) );
    INSTPAT( "DCP", 0xC7, ZEROPAGE, DCP( zeropage_addr, M ) );
    INSTPAT( "DCP", 0xD7, ZEROPAGE_X, DCP( zeropage_addr, M ) );
    INSTPAT( "DCP", 0xC3, INDEXED_INDIRECT, DCP( indirect_addr, M ) );
    INSTPAT( "DCP", 0xD3, INDIRECT_INDEXED, DCP( indirect_addr, M ) );

#define ISC_( addr_, M_ ) INC_( addr_, M_ ), SBC_( cpu.accumulator, M_ )
    INSTPAT( "ISC", 0xEF, ABSOLUTE, M = vaddr_read( absolute_addr ), ISC_( absolute_addr, M ) );
    INSTPAT( "ISC", 0xFF, ABSOLUTE_X, ISC_( absolute_addr, M ) );
    INSTPAT( "ISC", 0xFB, ABSOLUTE_Y, ISC_( absolute_addr, M ) );
    INSTPAT( "ISC", 0xE7, ZEROPAGE, ISC_( zeropage_addr, M ) );
    INSTPAT( "ISC", 0xF7, ZEROPAGE_X, ISC_( zeropage_addr, M ) );
    INSTPAT( "ISC", 0xE3, INDEXED_INDIRECT, ISC_( indirect_addr, M ) );
    INSTPAT( "ISC", 0xF3, INDIRECT_INDEXED, ISC_( indirect_addr, M ) );

#define SLO_( addr_, M_ ) ASL_M_( addr_, M_ ), ORA_( cpu.accumulator, M_ )
    INSTPAT( "SLO", 0x0F, ABSOLUTE, M = vaddr_read( absolute_addr ), SLO_( absolute_addr, M ) );
    INSTPAT( "SLO", 0x1F, ABSOLUTE_X, SLO_( absolute_addr, M ) );
    INSTPAT( "SLO", 0x1B, ABSOLUTE_Y, SLO_( absolute_addr, M ) );
    INSTPAT( "SLO", 0x07, ZEROPAGE, SLO_( zeropage_addr, M ) );
    INSTPAT( "SLO", 0x17, ZEROPAGE_X, SLO_( zeropage_addr, M ) );
    INSTPAT( "SLO", 0x03, INDEXED_INDIRECT, SLO_( indirect_addr, M ) );
    INSTPAT( "SLO", 0x13, INDIRECT_INDEXED, SLO_( indirect_addr, M ) );

#define RLA_( addr_, M_ ) ROL_( M_ ), vaddr_write( addr_, M_ ), AND_( M_ )
    INSTPAT( "RLA", 0x2F, ABSOLUTE, M = vaddr_read( absolute_addr ), RLA_( absolute_addr, M ) );
    INSTPAT( "RLA", 0x3F, ABSOLUTE_X, RLA_( absolute_addr, M ) );
    INSTPAT( "RLA", 0x3B, ABSOLUTE_Y, RLA_( absolute_addr, M ) );
    INSTPAT( "RLA", 0x27, ZEROPAGE, RLA_( zeropage_addr, M ) );
    INSTPAT( "RLA", 0x37, ZEROPAGE_X, RLA_( zeropage_addr, M ) );
    INSTPAT( "RLA", 0x23, INDEXED_INDIRECT, RLA_( indirect_addr, M ) );
    INSTPAT( "RLA", 0x33, INDIRECT_INDEXED, RLA_( indirect_addr, M ) );

#define SRE_( addr_, M_ ) LSR_M_( addr_, M_ ), EOR_( cpu.accumulator, M_ )
    INSTPAT( "SRE", 0x4F, ABSOLUTE, M = vaddr_read( absolute_addr ), SRE_( absolute_addr, M ) );
    INSTPAT( "SRE", 0x5F, ABSOLUTE_X, SRE_( absolute_addr, M ) );
    INSTPAT( "SRE", 0x5B, ABSOLUTE_Y, SRE_( absolute_addr, M ) );
    INSTPAT( "SRE", 0x47, ZEROPAGE, SRE_( zeropage_addr, M ) );
    INSTPAT( "SRE", 0x57, ZEROPAGE_X, SRE_( zeropage_addr, M ) );
    INSTPAT( "SRE", 0x43, INDEXED_INDIRECT, SRE_( indirect_addr, M ) );
    INSTPAT( "SRE", 0x53, INDIRECT_INDEXED, SRE_( indirect_addr, M ) );

#define RRA_( addr_, M_ ) ROR_( M_ ), vaddr_write( addr_, M_ ), ADC_( cpu.accumulator, M_, CARRY_ )
    INSTPAT( "RRA", 0x6F, ABSOLUTE, M = vaddr_read( absolute_addr ), RRA_( absolute_addr, M ) );
    INSTPAT( "RRA", 0x7F, ABSOLUTE_X, RRA_( absolute_addr, M ) );
    INSTPAT( "RRA", 0x7B, ABSOLUTE_Y, RRA_( absolute_addr, M ) );
    INSTPAT( "RRA", 0x67, ZEROPAGE, RRA_( zeropage_addr, M ) );
    INSTPAT( "RRA", 0x77, ZEROPAGE_X, RRA_( zeropage_addr, M ) );
    INSTPAT( "RRA", 0x63, INDEXED_INDIRECT, RRA_( indirect_addr, M ) );
    INSTPAT( "RRA", 0x73, INDIRECT_INDEXED, RRA_( indirect_addr, M ) );

#define UNOP( code_, MODE__ ) INSTPAT( "NOP", code_, MODE__, CYC( 0 ) )
    UNOP( 0x04, ZEROPAGE );
    UNOP( 0x14, ZEROPAGE_X );
    UNOP( 0x34, ZEROPAGE_X );
    UNOP( 0x44, ZEROPAGE );
    UNOP( 0x54, ZEROPAGE_X );
    UNOP( 0x64, ZEROPAGE );
    UNOP( 0x74, ZEROPAGE_X );
    UNOP( 0xD4, ZEROPAGE_X );
    UNOP( 0xF4, ZEROPAGE_X );

    UNOP( 0x80, IMMEDIATE );
    UNOP( 0x82, IMMEDIATE );
    UNOP( 0x89, IMMEDIATE );
    UNOP( 0xC2, IMMEDIATE );
    UNOP( 0xE2, IMMEDIATE );

    UNOP( 0x1A, IMPLICIT );
    UNOP( 0x3A, IMPLICIT );
    UNOP( 0x5A, IMPLICIT );
    UNOP( 0x7A, IMPLICIT );
    UNOP( 0xDA, IMPLICIT );
    UNOP( 0xFA, IMPLICIT );
    // UNOP( 0xEA, IMMEDIATE ); official...

    UNOP( 0x0C, ABSOLUTE_X );
    UNOP( 0x1C, ABSOLUTE_X );
    UNOP( 0x3C, ABSOLUTE_X );
    UNOP( 0x5C, ABSOLUTE_X );
    UNOP( 0x7C, ABSOLUTE_X );
    UNOP( 0xDC, ABSOLUTE_X );
    UNOP( 0xFC, ABSOLUTE_X );
}
