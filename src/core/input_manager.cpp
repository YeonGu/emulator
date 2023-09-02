//
// Created by 谢子南 on 2023/9/2.
//

#include "input_manager.h"


Gamepad_code keyboard_to_gamepadCode[256];

void keyboard_mapping(SDL_KeyCode keyCode,Gamepad_code gamepadCode)
{

}

uint8_t get_key_shift_reg_lsb()
{

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

        if(event.type == SDL_KEYDOWN)
        {

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

    keyboard_mapping(SDLK_UP,NES_A);
    keyboard_mapping(SDLK_RIGHT,NES_A);

    keyboard_mapping(SDLK_DOWN,NES_B);
    keyboard_mapping(SDLK_LEFT,NES_B);

    keyboard_mapping(SDLK_n,NES_Start);
    keyboard_mapping(SDLK_m,NES_Select);
}
