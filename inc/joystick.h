//
// Created by ZinanXie on 2023/9/11.
//

#ifndef EMULATOR_JOYSTICK_H
#define EMULATOR_JOYSTICK_H

#include <cstdint>
#include <cassert>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL.h>
#include <vector>

enum joystick_code{
    NES_A = 0,
    NES_B = 1,
    NES_Select = 2,
    NES_Start = 3,
    NES_Up = 4,
    NES_Down = 5,
    NES_Left = 6,
    NES_Right = 7
};

class joystick
{
private:
    std::vector<int8_t> keyboard_to_joystick;
    uint8_t       key_shift_reg = 0;
    uint8_t       key_shift_reg_copy;
    int  reg_bit;
public:
    joystick(){
        key_shift_reg_copy = 0;
        reg_bit = 0;
        keyboard_to_joystick = std::vector<int8_t>(256,-1);
    }
    void ready_to_get_shift_reg( uint16_t addr, uint8_t data )
    {
        //assert( addr == 0x4016 );
        reg_bit            = 0;
        key_shift_reg_copy = key_shift_reg;
    }

    uint8_t get_key_shift_reg_lsb( uint16_t addr )
    {
        //assert( addr == 0x4016 );
        if ( reg_bit >= 8 ) return 0;
        return ( ( key_shift_reg_copy >> reg_bit++ ) & 1 );
    }

    void keyboard_mapping( SDL_KeyCode keyCode, joystick_code gamepadCode )
    {
        assert( keyCode < 256 );
        keyboard_to_joystick[ keyCode ] = gamepadCode;
    }

    void enable_joystick_input(int index)
    {
        if( SDL_NumJoysticks() < 1 )
        {
            printf( "No joysticks connected!\n" );
        }
        else
        {
            printf("Found joysticks!\n");
            auto gGameController = SDL_JoystickOpen( index );
            if( gGameController == nullptr )
            {
                printf( "Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError() );
            }
        }
    }

    joystick& operator<<(SDL_Event& event)
    {
        if ( event.type == SDL_KEYDOWN || event.type == SDL_KEYUP || event.type == SDL_JOYAXISMOTION )
        {
            if ( event.key.keysym.sym >= 256 || keyboard_to_joystick[ event.key.keysym.sym ] < 0 ) return *this;
            key_shift_reg &= ~( 1 << keyboard_to_joystick[ event.key.keysym.sym ] );
            key_shift_reg |= ( event.type == SDL_KEYDOWN ? 1 : 0 ) << keyboard_to_joystick[ event.key.keysym.sym ];
        }else if(event.type == SDL_JOYBALLMOTION | event.type == SDL_JOYHATMOTION)
        {
            printf("joysti");
        }
        return *this;
    }
};

#endif //EMULATOR_JOYSTICK_H
