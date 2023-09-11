///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/21
//
///////////////////////////////////////////////////////////////////////
#define SDL_MAIN_HANDLED
#include "../inc/rom.h"
#include "input_manager.h"
#include "mmu.h"
#include "nes.h"
#include <chrono>
#include <cmath>
#include <cpu.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>

void  init_memory();
char *rom_path = nullptr;

const int AUDIO_AMP         = 28000;
const int AUDIO_SAMPLE_RATE = 44100;
const int AUDIO_DURATION    = 2;
void      sdl_audio_callback( void *userdata, Uint8 *stream, int len )
{
    //    Sint16 *samples     = reinterpret_cast<Sint16 *>( stream );
    //    int     sampleCount = len / sizeof( Sint16 );
    //    double  freq        = 440.0; // 频率为440Hz
    //
    //    for ( int i = 0; i < sampleCount; ++i )
    //    {
    //        double time  = static_cast<double>( i ) / AUDIO_SAMPLE_RATE;
    //        double value = AUDIO_AMP * sin( 2.0 * M_PI * freq * time );
    //        samples[ i ] = static_cast<Sint16>( value );
    //    }
    Sint16 *audio  = reinterpret_cast<Sint16 *>( stream );
    int     length = len / sizeof( Sint16 );

    auto     now     = std::chrono::system_clock::now().time_since_epoch();
    uint64_t time_ms = std::chrono::duration_cast<std::chrono::microseconds>( now ).count();
    double   t       = static_cast<double>( time_ms ) / 1000;

    double increment = 2.0 * M_PI * 440 / AUDIO_SAMPLE_RATE;
    double angle     = 0.0;

    for ( int i = 0; i < length; i++ )
    {
        static double pos;
        pos += 1.0 / AUDIO_SAMPLE_RATE;
        audio[ i ] = static_cast<Sint16>( AUDIO_AMP * sin( pos * 2.0 * 3.14159 * 440 ) );
        angle += increment;
    }
}

int main( int argc, char *argv[] )
{
    if ( argc >= 2 )
        rom_path = argv[ 1 ];

    SDL_setenv( "SDL_AUDIODRIVER", "directsound", 0 );
    if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO ) < 0 )
    {
        printf( "SDL初始化失败: %s\n", SDL_GetError() );
        return 1;
    }
    SDL_AudioSpec desiredSpec, obtainedSpec;
    desiredSpec.freq     = AUDIO_SAMPLE_RATE;
    desiredSpec.format   = AUDIO_S16;
    desiredSpec.channels = 1;
    desiredSpec.samples  = 2048;
    desiredSpec.callback = sdl_audio_callback;
    desiredSpec.userdata = nullptr;
    //        SDL_AudioDriverName( "directsound" );
    SDL_PauseAudio( 0 );
    if ( SDL_OpenAudio( &desiredSpec, &obtainedSpec ) != 0 )
    {
        std::cout << "Failed to open audio device: " << SDL_GetError() << std::endl;
        return 1;
    }
    SDL_PauseAudio( 0 );
    //    SDL_Delay( 2000 );

    init_mmu();
    init_input_manager();

    read_rom_mapper( argc, argv ); // Load ROM and INTERRUPT VECTORS ( nes_rom.c -> memory.c ); Init CPU cpu_memory_mapper
                                   // read_rom() -> read_info() -> init_cpu_mapper()
    init_memory();
    init_cpu(); // Init CPU (RESET vector). CPU Power up state...

    nes_mainloop();

    system( "pause" );
    return 0;
}