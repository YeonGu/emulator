//
// Created by 谢子南 on 2023/9/2.
//

#ifndef EMULATOR_INPUT_MANAGER_H
#define EMULATOR_INPUT_MANAGER_H

#include <cstdint>
#include <SDL2/SDL.h>

enum Gamepad_code{
    NES_A = 0,
    NES_B = 1,
    NES_Select = 2,
    NES_Start = 3,
    NES_Up = 4,
    NES_Down = 5,
    NES_Left = 6,
    NES_Right = 7
};

void init_input_manager();

void keyboard_mapping(SDL_KeyCode keyCode,Gamepad_code gamepadCode);

void ready_to_get_shift_reg(uint16_t addr,uint8_t data);
uint8_t get_key_shift_reg_lsb(uint16_t addr);

void scan_input();

#endif //EMULATOR_INPUT_MANAGER_H
