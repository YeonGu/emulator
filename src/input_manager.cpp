//
// Created by 谢子南 on 2023/9/2.
//

#include "input_manager.h"
#include "joystick.h"
#include <assert.h>
#include "mmu.h"
#include <cstdint>
#include <cstdio>


joystick joy[2];

void scan_input()
{
    SDL_Event event;
    while ( SDL_PollEvent( &event ) )
    {
        if ( event.type == SDL_QUIT ) [[unlikely]]
        {
            exit( 0 );
        }
        joy[0] << event;
        joy[1] << event;
    }
}

void init_input_manager()
{

    joy[0].keyboard_mapping( SDLK_w, NES_Up );
    joy[0].keyboard_mapping( SDLK_a, NES_Left );
    joy[0].keyboard_mapping( SDLK_d, NES_Right );
    joy[0].keyboard_mapping( SDLK_s, NES_Down );

    joy[0].keyboard_mapping( SDLK_j, NES_A );
    //joy[0].keyboard_mapping( SDLK_p, NES_A );

    joy[0].keyboard_mapping( SDLK_k, NES_B );
    joy[0].keyboard_mapping( SDLK_l, NES_B );

    joy[0].keyboard_mapping( SDLK_i, NES_Start );
    joy[0].keyboard_mapping( SDLK_u, NES_Select );
    get_mmu()->mmap(0x4016, nullptr,1,
                    std::bind(&joystick::get_key_shift_reg_lsb,&joy[0],std::placeholders::_1),
                    std::bind(&joystick::ready_to_get_shift_reg,&joy[0],std::placeholders::_1,std::placeholders::_2)
                    );
    get_mmu()->mmap(0x4017, nullptr,1,
                    std::bind(&joystick::get_key_shift_reg_lsb,&joy[1],std::placeholders::_1),
                    std::bind(&joystick::ready_to_get_shift_reg,&joy[1],std::placeholders::_1,std::placeholders::_2)
    );

    joy[0].enable_joystick_input(0);
}
