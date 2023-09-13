//
// Created by 顾雨杭 on 2023/9/12.
//

#ifndef EMULATOR_APU_REGS_H
#define EMULATOR_APU_REGS_H

#include <cstdint>
// =========================================================================================
// 4000 - 4007 Pulse
struct apu_pulse_ctrl_t
{
    uint8_t vol : 4;
    uint8_t const_vol : 1;
    uint8_t length_cnt_halt : 1;
    uint8_t duty : 2;
};
struct apu_pulse_sweep_t
{
    uint8_t shift : 3;
    uint8_t negate : 1;
    uint8_t period : 3;
    uint8_t enabled : 1;
};
struct apu_pulse_length_cnt_t
{
    uint8_t timer_high : 3;
    uint8_t length_cnt_ld : 5;
};

struct apu_triangle_ctrl_t
{
    uint8_t linear_cnt_ld : 7;
    uint8_t length_cnt_halt : 1;
};
struct apu_triangle_length_cnt_t
{
    uint8_t timer_hi : 3;
    uint8_t length_cnt_ld : 5;
};

struct apu_pulse_channel_t
{
    apu_pulse_ctrl_t       ctrl;
    apu_pulse_sweep_t      sweep;
    uint8_t                timer_lo;
    apu_pulse_length_cnt_t length_cnt;
};
struct apu_triangle_channel_t
{
    apu_triangle_ctrl_t       ctrl;
    uint8_t                   unused;
    uint8_t                   timer_lo;
    apu_triangle_length_cnt_t length_cnt;
};
// =========================================================================================
// 4015: Status
// $4015 write	---D NT21	Enable DMC (D), noise (N), triangle (T), and pulse channels (2/1)
struct apu_status_write_t
{
    uint8_t pulse_1 : 1;
    uint8_t pulse_2 : 1;
    uint8_t triangle : 1;
    uint8_t noise : 1;
    uint8_t dmc : 1;
    uint8_t unused : 3;
};
// $4015 read	IF-D NT21	DMC interrupt (I), frame interrupt (F), DMC active (D), length counter > 0 (N/T/2/1)
struct apu_status_read_t
{
    uint8_t pulse1 : 1;
    uint8_t pulse2 : 1;
    uint8_t triangle : 1;
    uint8_t noise : 1;
    uint8_t dmc_active : 1;
    uint8_t unused : 1;
    uint8_t frame_interrupt : 1;
    uint8_t dmc_interrupt : 1;
};
struct apu_frame_cnt_t
{
    uint8_t unused : 6;
    uint8_t irq_inhibit : 1;
    uint8_t mode : 1;
};
#endif // EMULATOR_APU_REGS_H
