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
void render_bg( uint32_t *vmem );

SDL_Window   *window;
SDL_Renderer *renderer;
SDL_Texture  *texture;
SDL_Surface  *surface;

static uint32_t vmem[ 256 * 240 ];

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

    //    set_ppu_nmi( true );
    //    cpu_call_interrupt();
    //    cpu_exec( 300 );
    //    set_ppu_nmi( false );

    int i = 50;
    while ( i-- )
    {
        cpu_exec( 20000 );
        set_ppu_nmi( true );

        printf( "cpu enter int %d\n", 100 - i );
        cpu_call_interrupt();
        //        cpu_exec( 1400 );
        //        set_ppu_nmi( false );
        //        cpu_exec( 400 );

        render_bg( vmem );
        SDL_UpdateTexture( texture, NULL, vmem, 256 * sizeof( uint32_t ) );
        SDL_RenderClear( renderer );
        SDL_RenderCopy( renderer, texture, NULL, NULL );
        SDL_RenderPresent( renderer );
    }
    cpu_exec( 1000 );
    system( "pause" );
    printf( "Exit NES mainloop.\n" );
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
    SDL_Window *sdlwindow = SDL_CreateWindow(
        "SDL窗口",               // 窗口标题
        SDL_WINDOWPOS_UNDEFINED, // 窗口的初始位置
        SDL_WINDOWPOS_UNDEFINED,
        256,             // 窗口的宽度
        240,             // 窗口的高度
        SDL_WINDOW_SHOWN // 窗口的显示标志
    );

    if ( !sdlwindow )
    {
        printf( "窗口创建失败: %s\n", SDL_GetError() );
        return 1;
    }

    renderer = SDL_CreateRenderer( sdlwindow, -1, SDL_RENDERER_ACCELERATED );
    assert( renderer );

    //    surface = SDL_CreateRGBSurfaceWithFormatFrom( vmem, 256, 240, 32, 256 * 4, SDL_PIXELFORMAT_RGBA32 );
    //    assert( surface );
    texture = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, 256, 240 );
    assert( texture );
    // 等待窗口关闭
    //    bool      quit = false;
    //    SDL_Event event;
    //
    //    while ( !quit )
    //    {
    //        while ( SDL_PollEvent( &event ) )
    //        {
    //            if ( event.type == SDL_QUIT )
    //            {
    //                quit = true;
    //            }
    //        }
    //    }
    //
    //    // 清理SDL
    //    SDL_DestroyWindow( sdlwindow );
    //    SDL_Quit();

    return 0;
}