///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/9/12
//
//  pulse.cpp
//
///////////////////////////////////////////////////////////////////////

#include <apu.h>

pulse_channel::pulse_channel()
    : enable( false )
    , freq( 0 )
    , duration( 0 )
{
    reg_entry[ 0 ] = reinterpret_cast<uint8_t *>( &ctrl );
    reg_entry[ 1 ] = reinterpret_cast<uint8_t *>( &sweep );
    reg_entry[ 2 ] = &timer_lo;
    reg_entry[ 3 ] = reinterpret_cast<uint8_t *>( &length );
}

// Write to Pulse channel
void pulse_channel::reg_write( int index, byte data )
{
    //    printf( "PULSE REGISTER WRITE %d = %02x\n", index, data );
    auto update_freq = [ this ]() {
        next_info.used  = true;
        next_info.timer = timer;
        next_info.freq  = 1789773.0 / 16 / ( 1 + timer );
    };
    auto update_duty = [ this ]() {
        next_info.used = true;
        next_info.duty = ctrl.duty; // 2bits (DD)
    };
    auto update_waveform = [ this ]() {
        if ( !next_info.used ) return;
        next_info.used = false;
    };
    auto update_duration = [ this ]() {
        uint8_t nes_length = length.length_cnt_ld;
        double  len        = (double) nes_length * 751000 / ( get_apu()->fc_mode ? 60 : 48 );
        duration           = (int) len;
        //        printf( "NES Length = %d, duration = %d\n", nes_length, duration );
    };

    index %= 4;
    *reg_entry[ index ] = data;
    switch ( index )
    {
    case 0: // DDLC VVVV Duty (D), envelope loop / length counter halt (L), constant volume (C), volume/envelope (V)
        duty          = ctrl.duty;
        infinite_play = ctrl.length_cnt_halt;
        const_vol     = ctrl.const_vol;
        volume        = ctrl.vol;
        // reset current_pulse
        update_duty();
        break;
    case 1: // 	EPPP NSSS	Sweep unit: enabled (E), period (P), negate (N), shift (S)
        break;
    case 2: // 	TTTT TTTT	Timer low (T)
        timer &= 0xFF00;
        timer |= timer_lo;
        // reset current_pulse
        update_freq();
        break;
    case 3:
        timer &= 0x00FF;
        timer |= (uint16_t) length.timer_high << 8;
        // reset current_pulse
        //        printf( "4003 = %02x\n", *reinterpret_cast<uint8_t *>( &length ) );
        update_freq();
        update_duration();
        break;
    default:
        break;
    }
}
byte pulse_channel::reg_read( int index )
{
    //    printf( "PULSE REGISTER READ %d\n", index );
    return *reg_entry[ index ];
}
byte pulse_channel::get_next_sample()
{
    auto reload_info = [ this ]() {
        cursor             = 0;
        current_pulse.end  = static_cast<uint32_t>( ( 44100.0 / next_info.freq ) );
        current_pulse.rise = current_pulse.end / 2; // TODO: more duty...
    };

    uint8_t data;
    if ( cursor < current_pulse.rise )
    {
        data = 0;
    }
    else
    {
        if ( !infinite_play && !duration )
            data = 0;
        else
            data = volume;
    }
    if ( !enable ) data = 0;

    if ( !infinite_play && duration )
        duration--;

    if ( cursor >= current_pulse.end )
        reload_info();
    else
        cursor++;

    return data;
}
