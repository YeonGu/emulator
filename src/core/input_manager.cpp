//
// Created by 谢子南 on 2023/9/2.
//

#include "input_manager.h"
#include <assert.h>

unsigned char keyboard_to_gamepadCode[256];
uint8_t key_shift_reg = 0;
uint8_t key_shift_reg_copy;

void keyboard_mapping(SDL_KeyCode keyCode,Gamepad_code gamepadCode)
{
    assert(keyCode < 256);
    keyboard_to_gamepadCode[keyCode] = gamepadCode;
}

int reg_bit = 0;
void ready_to_get_shift_reg(uint16_t addr,uint8_t data)
{
    assert(addr == 0x4016);
    reg_bit = 0;
    key_shift_reg_copy = key_shift_reg;
    printf("will read key\n");
}


uint8_t get_key_shift_reg_lsb(uint16_t addr)
{
    assert(addr == 0x4016);
    if(reg_bit >= 8)reg_bit=0;
    printf("read key bit %d is %d\n",reg_bit,(key_shift_reg_copy >> reg_bit )& 1);
    return ((key_shift_reg_copy >> reg_bit++ )& 1);
}

void scan_input()
{
    SDL_Event event;
    while ( SDL_PollEvent( &event ) )
    {
        if ( event.type == SDL_QUIT ) [[unlikely]]
        {
            exit( 0 );
        }

        if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
        {
            if(keyboard_to_gamepadCode[event.key.keysym.sym] < 0)return;
            key_shift_reg &= ~(1 << keyboard_to_gamepadCode[event.key.keysym.sym]);
            key_shift_reg |= (event.type == SDL_KEYDOWN ? 1 : 0) << keyboard_to_gamepadCode[event.key.keysym.sym];
            printf("set key %d as %d\n",keyboard_to_gamepadCode[event.key.keysym.sym],(event.type == SDL_KEYDOWN ? 1 : 0));
        }

    }
}


void init_input_manager()
{
    memset(keyboard_to_gamepadCode,1,sizeof keyboard_to_gamepadCode);
    keyboard_mapping(SDLK_w,NES_Up);
    keyboard_mapping(SDLK_a,NES_Left);
    keyboard_mapping(SDLK_d,NES_Right);
    keyboard_mapping(SDLK_s,NES_Down);

    keyboard_mapping(SDLK_o,NES_A);
    keyboard_mapping(SDLK_p,NES_A);

    keyboard_mapping(SDLK_k,NES_B);
    keyboard_mapping(SDLK_l,NES_B);

    keyboard_mapping(SDLK_n,NES_Start);
    keyboard_mapping(SDLK_m,NES_Select);
}
