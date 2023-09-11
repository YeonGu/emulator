//
// Created by ZinanXie on 2023/9/11.
//

#ifndef EMULATOR_JOYSTICK_H
#define EMULATOR_JOYSTICK_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <vector>

enum joystick_code
{
    NES_A      = 0,
    NES_B      = 1,
    NES_Select = 2,
    NES_Start  = 3,
    NES_Up     = 4,
    NES_Down   = 5,
    NES_Left   = 6,
    NES_Right  = 7
};

enum DUALSHOCK4
{
    DS4_X        = 0,
    DS4_O        = 1,
    DS4_Q        = 2, // []
    DS4_A        = 3, // ∆
    DS4_SHARE    = 4,
    DS4_OPTHIONS = 6,
    DS4_LA       = 7,
    DS4_RA       = 8,
    DS4_L1       = 9,
    DS4_R1       = 10,
    DS4_Up       = 11,
    DS4_Down     = 12,
    DS4_Left     = 13,
    DS4_Right    = 14,
    DS4_TOUCHBAR = 15,
};

#define DualShock 0
#define Xbox 1

class joystick
{
  private:
    std::vector<int8_t> keyboard_to_joystick;
    std::vector<int8_t> phy_joystick_to_joystick;
    uint8_t             key_shift_reg = 0;
    uint8_t             key_shift_reg_copy;
    int8_t              joystick_index;
    int                 joy_type;
    int                 reg_bit;

    void phy_joystick_mapping( int8_t ds4_key, joystick_code joystickCode )
    {
        phy_joystick_to_joystick[ ds4_key ] = joystickCode;
    }

    void set_shift_reg( int bit, uint8_t value )
    {
        key_shift_reg &= ~( 1 << bit );
        key_shift_reg |= !!value << bit;
    }

    uint8_t get_shift_reg( int bit )
    {
        return key_shift_reg >> bit & 0x1;
    }

  public:
    joystick()
    {
        key_shift_reg_copy       = 0;
        reg_bit                  = 0;
        keyboard_to_joystick     = std::vector<int8_t>( 256, -1 );
        phy_joystick_to_joystick = std::vector<int8_t>( 256, -1 );
        joystick_index           = -1;
    }
    void ready_to_get_shift_reg( uint16_t addr, uint8_t data )
    {
        // assert( addr == 0x4016 );
        reg_bit            = 0;
        key_shift_reg_copy = key_shift_reg;
    }

    uint8_t get_key_shift_reg_lsb( uint16_t addr )
    {
        // assert( addr == 0x4016 );
        if ( reg_bit >= 8 ) return 0;
        return ( ( key_shift_reg_copy >> reg_bit++ ) & 1 );
    }

    void keyboard_mapping( SDL_KeyCode keyCode, joystick_code gamepadCode )
    {
        assert( keyCode < 256 );
        keyboard_to_joystick[ keyCode ] = gamepadCode;
    }

    void enable_joystick_input( int8_t index, int _joy_type = DualShock )
    {
        if ( SDL_NumJoysticks() < 1 )
        {
            printf( "No joysticks connected!\n" );
        }
        else
        {
            printf( "Found joysticks!\n" );
            auto gGameController = SDL_JoystickOpen( index );
            if ( gGameController == nullptr )
            {
                printf( "Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError() );
            }
        }
        joystick_index = index;

        this->joy_type = _joy_type;

        switch ( _joy_type )
        {
        case DualShock:
            phy_joystick_mapping( DS4_Up, NES_Up );
            phy_joystick_mapping( DS4_Down, NES_Down );
            phy_joystick_mapping( DS4_Left, NES_Left );
            phy_joystick_mapping( DS4_Right, NES_Right );
            phy_joystick_mapping( DS4_SHARE, NES_Start );
            phy_joystick_mapping( DS4_OPTHIONS, NES_Select );
            phy_joystick_mapping( DS4_X, NES_A );
            phy_joystick_mapping( DS4_O, NES_B );
            break;
        case Xbox:
            phy_joystick_mapping( 0, NES_A );
            phy_joystick_mapping( 1, NES_B );
            phy_joystick_mapping( 4, NES_Select );
            phy_joystick_mapping( 5, NES_Start );

            phy_joystick_mapping( 14, NES_Up );    // 1
            phy_joystick_mapping( 15, NES_Right ); // 2
            phy_joystick_mapping( 17, NES_Down );  // 4
            phy_joystick_mapping( 21, NES_Left );  // 8
            break;
        default:

            break;
        }
    }

    joystick &operator<<( SDL_Event &event )
    {
        if ( event.type == SDL_KEYDOWN || event.type == SDL_KEYUP || event.type == SDL_JOYAXISMOTION )
        {
            if ( event.key.keysym.sym >= 256 || keyboard_to_joystick[ event.key.keysym.sym ] < 0 )
                return *this;
            set_shift_reg( keyboard_to_joystick[ event.key.keysym.sym ], event.type == SDL_KEYDOWN );
        }
        else if ( event.type == SDL_JOYBUTTONDOWN || event.type == SDL_JOYBUTTONUP || event.type == SDL_JOYHATMOTION )
        {
            printf( "button %d is %d\n", event.jbutton.button, event.jbutton.state );
            if ( event.jbutton.which != joystick_index | phy_joystick_to_joystick[ event.jbutton.button ] < 0 )
                return *this;
            if ( joy_type == DualShock )
            {
                set_shift_reg( phy_joystick_to_joystick[ event.jbutton.button ], event.jbutton.state );
            }
            else if ( joy_type == Xbox )
            {
                if ( event.type == SDL_JOYHATMOTION )
                {
                    for ( int _bit = 0; _bit < 4; _bit++ )
                    {
                        set_shift_reg( phy_joystick_to_joystick[ ( 1 << _bit ) + 13 ], ( event.jbutton.state >> _bit ) & 0x1 );
                    }
                }
                else
                {
                    set_shift_reg( phy_joystick_to_joystick[ event.jbutton.button ], event.jbutton.state );
                }
            }
        }
        return *this;
    }
};

#endif // EMULATOR_JOYSTICK_H
