///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/27
//
//  nes.c
//
///////////////////////////////////////////////////////////////////////

#include <SDL2/SDL.h>
#include <cpu.h>
#include <ppu-reg.h>
#include <stdio.h>
void cpu_call_interrupt();
int  sdl_test();

void nes_mainloop()
{
    //    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    //    {
    //        printf( "SDL Init failed" );
    //        return;
    //    }
    //    printf( "SDL Init succeeded" );
    sdl_test();
    printf( "Entered NES mainloop.\n" );
    cpu_exec( 15000 );

    set_ppu_nmi_enable( true );

    set_ppu_nmi( true );
    cpu_call_interrupt();
    cpu_exec( 300 );
    set_ppu_nmi( false );

    printf( "\n\nCYCLE 1....\n\n" );

    int i = 10;
    while ( i-- )
    {
        cpu_exec( 10000 );
        set_ppu_nmi( true );
        cpu_call_interrupt();
        cpu_exec( 300 );
        set_ppu_nmi( false );
    }

    printf( "Exited NES mainloop.\n" );
}

int sdl_test()
{
    // 初始化SDL
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "SDL初始化失败: %s\n", SDL_GetError() );
        return 1;
    }

    // 创建窗口
    SDL_Window *window = SDL_CreateWindow(
        "SDL窗口",               // 窗口标题
        SDL_WINDOWPOS_UNDEFINED, // 窗口的初始位置
        SDL_WINDOWPOS_UNDEFINED,
        640,             // 窗口的宽度
        480,             // 窗口的高度
        SDL_WINDOW_SHOWN // 窗口的显示标志
    );

    if ( !window )
    {
        printf( "窗口创建失败: %s\n", SDL_GetError() );
        return 1;
    }

    // 等待窗口关闭
    bool      quit = false;
    SDL_Event event;

    while ( !quit )
    {
        while ( SDL_PollEvent( &event ) )
        {
            if ( event.type == SDL_QUIT )
            {
                quit = true;
            }
        }
    }

    // 清理SDL
    SDL_DestroyWindow( window );
    SDL_Quit();

    return 0;
}