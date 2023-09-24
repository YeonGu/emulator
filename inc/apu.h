//
// Created by 顾雨杭 on 2023/9/11.
//

#ifndef EMULATOR_APU_H
#define EMULATOR_APU_H

#include <apu-regs.h>
#include <array>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <cmath>

using addr_t = uint16_t;
using byte   = uint8_t;

void fill_buf_callback( uint8_t *stream, int len );

class sweep
{
};
class envelope
{
};

class pulse_channel
{
  public:
    bool enable; // set by apu

  private:
    double   freq;
    uint8_t  duty;
    uint8_t  volume;
    bool     infinite_play;
    uint16_t timer;
    int      duration; // timer
    uint32_t cursor;
    struct pulse_info_t
    {
        uint32_t rise;
        uint32_t end;
    } current_pulse;
    struct next_pulse_t
    {
        bool     used;
        double   freq;
        uint16_t timer;
        uint8_t  duty;
    } next_info{.freq = NAN};

    bool const_vol;

  private:
    // 4 IO registers
    apu_pulse_ctrl_t       ctrl{};
    apu_pulse_sweep_t      sweep{};
    uint8_t                timer_lo{};
    apu_pulse_length_cnt_t length{};
    uint8_t               *reg_entry[ 4 ]{};

    class sweep pulse_sweep;
    envelope    pulse_envelope;

  public:
    pulse_channel();

    void reg_write( int index, byte data );
    byte reg_read( int index );
    byte get_next_sample();
};

class APU
{
  public:
    bool fc_mode; // $4017 bit 7(M); M0 = 4 step; M1 = 5 step
  private:
    uint64_t cpu_ticks;
    uint64_t apu_ticks;

    const int step_cnt[ 5 ] = { 3729, 7456, 11186, 14195, 18641 };
    bool      interrupt_inhibit; // Interrupt inhibit flag. If set, the frame interrupt flag is cleared, otherwise it is unaffected.
    int       frame_counter;

    uint8_t frame_int;

  private:
    //    apu_pulse_channel_t    pulse_reg[ 2 ];
    //    apu_triangle_channel_t triangle_reg;
    // Noise
    // DMC
    apu_status_read_t status_reg;
    apu_frame_cnt_t   frame_cnt;

    pulse_channel pulse1;
    pulse_channel pulse2;

    std::vector<std::function<byte()>> channel_sampling;

    struct reg_handler_t
    {
        std::function<void( byte )> _write_b = []( byte data ) {};
        std::function<byte()>       _read_b  = []() { return 0; };
    } reg_handler[ 0x18 ];
    byte io_regs[ 0x18 ];

    void set_reg_handlers();

  public:
    APU();
    void tick();

    void apu_reg_write( addr_t addr, byte data )
    {
        //        printf( "APU IO REG WRITE %04x = %02x\n", addr, data );
        reg_handler[ addr - 0x4000 ]._write_b( data );
    };
    byte apu_reg_read( addr_t addr )
    {
        auto data = reg_handler[ addr - 0x4000 ]._read_b();
        //        printf( "APU IO REG read %04x = %02x\n", addr, data );
        return data;
    }

    byte get_next_sample();
};
APU *get_apu();
#endif // EMULATOR_APU_H
