//
// Created by 谢子南 on 2023/9/2.
//

#ifndef EMULATOR_INPUT_MANAGER_H
#define EMULATOR_INPUT_MANAGER_H

#include <cstdint>
#include <SDL2/SDL.h>

enum Gamepad_code{
    NES_A,
    NES_B,
    NES_Select,
    NES_Start,
    NES_Up,
    NES_Down,
    NES_Left,
    NES_Right
};

void init_input_manager();

void keyboard_mapping(SDL_KeyCode keyCode,Gamepad_code gamepadCode);

uint8_t get_key_shift_reg_lsb();

void scan_input();

#endif //EMULATOR_INPUT_MANAGER_H
