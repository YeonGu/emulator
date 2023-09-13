///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/9/11
//
//  apu.cpp
//
///////////////////////////////////////////////////////////////////////

//
// Created by 顾雨杭 on 2023/9/11.
//

#include <apu.h>

// Singleton
APU *apu_inst;
APU *get_apu()
{
    if ( !apu_inst )
        apu_inst = new APU();
    return apu_inst;
}
APU::APU()
{
    apu_inst = this;
    set_reg_handlers();
    channel_sampling.emplace_back( [ this ]() { return pulse1.get_next_sample(); } );
    channel_sampling.emplace_back( [ this ]() { return pulse2.get_next_sample(); } );
}

#define FC_4STEP false
#define FC_5STEP true

void cpu_call_irq();
void APU::tick()
{
    if ( !( cpu_ticks++ & 0x01 ) )
        return;

    // APU Tick
    if ( frame_counter == step_cnt[ 0 ] )
    {
    }
    if ( frame_counter == step_cnt[ 1 ] )
    {
    }
    if ( frame_counter == step_cnt[ 2 ] )
    {
    }
    if ( frame_counter == step_cnt[ 3 ] )
    {
        if ( fc_mode == FC_4STEP && !interrupt_inhibit )
        {
            frame_int = 1;
            cpu_call_irq();
        }
    }
    if ( frame_counter == step_cnt[ 4 ] )
    {
    }

    apu_ticks++;
    if ( !fc_mode ) // 4 step
    {
        frame_counter = ( frame_counter == step_cnt[ 3 ] ) ? 0 : frame_counter + 1;
    }
    else // 5 step
    {
        frame_counter = ( frame_counter == step_cnt[ 4 ] ) ? 0 : frame_counter + 1;
    }
}

void APU::set_reg_handlers()
{
    // $4000 - $4003
    for ( int i = 0; i < 4; ++i )
    {
        reg_handler[ i ]._write_b = [ i, this ]( byte data ) {
            pulse1.reg_write( i, data );
        };
        reg_handler[ i ]._read_b = [ i, this ]() {
            return pulse1.reg_read( i );
        };
    }
    // $4004 - $4007
    for ( int i = 4; i < 8; ++i )
    {
        reg_handler[ i ]._write_b = [ i, this ]( byte data ) {
            pulse2.reg_write( i - 4, data );
        };
        reg_handler[ i ]._read_b = [ i, this ]() {
            return pulse1.reg_read( i - 4 );
        };
    }

    reg_handler[ 0x15 ]._write_b = [ this ]( byte data ) {
        io_regs[ 0x15 ] = data & 0x1F;
        // TODO: enable corresponding channels
        pulse1.enable = data & 0x01;
        pulse2.enable = data & 0x02;
    };
    reg_handler[ 0x15 ]._read_b = [ this ]() {
        auto data = io_regs[ 0x15 ]; // TODO
        data |= ( frame_int ? 1 : 0 ) << 6;
        return data;
    };

    // Frame Counter ($4017)
    //     See APU Frame Counter
    //         $4017	MI-- ----	Mode (M, 0 = 4-step, 1 = 5-step), IRQ inhibit flag (I)
    //         The frame counter is controlled by register $4017, and it drives the
    //         envelope, sweep, and length units on the pulse, triangle and noise channels.
    //         It ticks approximately 4 times per frame (240Hz NTSC), and executes either a 4 or 5-step sequence,
    //         depending on how it is configured. It may optionally issue an IRQ on the last tick of the 4-step sequence.
    // mode 0:    mode 1:       function
    // ---------  -----------  -----------------------------
    //  - - - f    - - - - -    IRQ (if bit 6 is clear)
    //  - l - l    - l - - l    Length counter and sweep
    //  e e e e    e e e - e    Envelope and linear counter
    reg_handler[ 0x17 ]._write_b = [ this ]( byte data ) {
        io_regs[ 0x17 ]   = data;
        fc_mode           = data & 0x80;
        interrupt_inhibit = data & 0x40;
    };
    reg_handler[ 0x17 ]._read_b = [ this ]() {
        return io_regs[ 0x17 ];
    };
}
byte APU::get_next_sample()
{
    //    std::vector<byte> sampling;
    //    sampling.reserve( channel_sampling.size() );
    int sum = 0; // FIXME
    for ( const auto &i : channel_sampling )
    {
        //        sampling.emplace_back( i() );
        sum += i();
    }
    return sum;
}

// void fill_buf_callback( uint8_t *stream, int len ) {}
