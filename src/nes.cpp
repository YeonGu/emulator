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
#include <cstdio>
#include <input_manager.h>
#include <ppu.h>
// #include <ppu-reg.h>

void cpu_call_interrupt();
int  sdl_test();
void render_bg( uint32_t *vmem );

SDL_Window   *window;
SDL_Renderer *renderer;
SDL_Texture  *texture;
SDL_Surface  *surface;

static uint32_t vmem[ 256 * 240 ];
uint8_t        *get_chr_rom( int idx );

void test_loop();
void nes_mainloop()
{
    ppu_inst = new ppu( get_chr_rom( 0 ), HORIZON_SCREEN );

    sdl_test();
    printf( "Entered NES mainloop.\n" );
    cpu_exec( 39900 );

    set_ppu_nmi_enable( true );
    test_loop();

    system( "pause" );
    printf( "Exit NES mainloop.\n" );
}

void test_loop()
{
    int test_times = 30;
    while ( 1 )
    {
        int cyc = 29781;
        while ( cyc-- )
        {
            ppu_inst->step( vmem );
            ppu_inst->step( vmem );
            ppu_inst->step( vmem );
            cpu_step();
        }

        ppu_inst->render_bg( vmem );
        SDL_UpdateTexture( texture, nullptr, vmem, 256 * sizeof( uint32_t ) );
        SDL_RenderClear( renderer );
        SDL_RenderCopy( renderer, texture, nullptr, nullptr );
        SDL_RenderPresent( renderer );
        scan_input();
    }

    // Only for test: print the ciram content
    for ( auto i : ppu_inst->ciram_0 )
    {
        static int addr = 0x2000;
        printf( "%02x ", i );
        addr++;
        if ( !( addr % 0x20 ) )
            printf( "\n" );
    }
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
        "NES EMULATOR",          // 窗口标题
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