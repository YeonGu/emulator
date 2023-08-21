///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/21
//
///////////////////////////////////////////////////////////////////////

#include "rom.h"
#include <assert.h>
#include <stdio.h>

char *rom_file;

int read_nes_rom( int argc, char **argv )
{
    // TODO: get the rom file path...
    rom_file = "E:\\0 SEU\\2023\\TiNES\\emulator\\rom\\mario.nes";
    if ( argc == 1 )
        printf( "No file target is given. Use the default mario file.\n" );

    FILE *file;

    file = fopen( rom_file, "r" );
    assert( file );
    struct nes_romhdr_t hdr;

    fread( &hdr, sizeof( hdr ), 1, file );
    assert( hdr.ident[ 0 ] == 'N' && hdr.ident[ 1 ] == 'E' && hdr.ident[ 2 ] == 'S' );
    printf( "NES ROM HDR check finished.\n" );

    fclose( file );

    return 0;
}