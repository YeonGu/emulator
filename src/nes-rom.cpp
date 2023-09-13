///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/8/21
//
///////////////////////////////////////////////////////////////////////

#include "memory.h"
#include "rom.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

const char           *rom_file;
struct nes_romhdr_t   rom_hdr;
struct nes_rom_info_t rom_info;

static long prg_size;
static long chr_size;

static uint8_t prg_rom_0[ 1 * 16 * 1024 ];
static uint8_t prg_rom_1[ 1 * 16 * 1024 ];
static uint8_t chr_rom[ 1 * 8 * 1024 ];

uint8_t *get_prg_rom( int idx )
{
    switch ( idx )
    {
    case 0:
        return prg_rom_0;
    case 1:
        return prg_rom_1;
    default:
        printf( "READ INVALID PRGROM %d\n", idx );
        assert( 0 );
    }
    return NULL;
}

uint8_t *get_chr_rom( int idx )
{
    assert( idx == 0 );
    return chr_rom;
}

extern char *rom_path;

void read_rom_info();
int  read_rom_mapper( int argc, char **argv )
{
    //    rom_file = "../rom-fix/test_cpu_exec_space_ppuio.nes"; // FIXME
    //    rom_file = "../rom-test/ppu/power_up_palette.nes"; // FIXME
    //    rom_file = "../rom-test/cpu_interrupts_v2/rom_singles/1-cli_latency.nes"; // FIXME APU...
    //    rom_file = "../rom-test/instr_test-v5/rom_singles/04-zero_page.nes"; // FIXME

    //    rom_file = "../rom-fix/nestest.nes"; // pass
    //    rom_file = "../rom-test/ppu/palette_ram.nes"; // pass
    //    rom_file = "../rom-test/ppu/vbl_clear_time.nes"; // pass
    //    rom_file = "../rom-test/ppu/vram_access.nes";
    //    rom_file = "../rom-test/ppu/sprite_ram.nes";
    //    rom_file = "../rom-fix/test_cpu_flag_concurrency.nes"; // pass
    //    rom_file = "../rom/f1.nes";
    //    rom_file = "../rom/kungfu.nes";
    //    rom_file = "../rom/pacman.nes";
    //    rom_file = "../rom/circus.nes";
    //    rom_file = "../rom/Donkey Kong (Japan).nes";
    if ( !rom_path )
    {
        rom_file = "rom/mario.nes";
    }
    else
    {
        rom_file = rom_path + 13;
    }

    if ( argc == 1 )
        printf( "No file target is given. Use the default mario file.\n" );
    FILE *file;
    file = fopen( rom_file, "rb+" );

    if ( !file )
    {
        FILE *log_file = fopen( "tinyneslog.txt", "w+" );
        fprintf_s( log_file, "ERROR: No valid file is given. the current argument: %s\nexe absolute path at %s", rom_file, _pgmptr );
        fclose( log_file );
        assert( 0 );
    }

    // Check rom ident
    fread( &rom_hdr, sizeof( rom_hdr ), 1, file );
    assert( rom_hdr.ident[ 0 ] == 'N' && rom_hdr.ident[ 1 ] == 'E' && rom_hdr.ident[ 2 ] == 'S' );
    printf( "NES ROM HDR check finished.\n\n" );

    // Read rom information
    read_rom_info();
    bool trainer = false;
    // FIXME: The Trainer Area follows the 16-byte Header and precedes the PRG-ROM area if bit 2 of Header byte 6 is set.
    // TODO: Trainer Area load
    fseek( file, trainer ? 16 + 512 : 16, SEEK_SET );

    // Fetch PRG-ROM, CHR-ROM
    fetch_rom( file, &rom_info );
    init_mapper( &rom_info ); // Init CPU ROM Mapper

    print_rom_info();
    fclose( file );
    return 0;
}

// TODO: init prg/chr fault process
void fetch_rom( FILE *file, struct nes_rom_info_t *info )
{
    if ( info->mapper != 0 )
    {
        printf( "ERROR: cpu_memory_mapper = %d, NOT IMPLEMENTED.", info->mapper );
        assert( 0 );
    }
    assert( ( info->prg_size == 1 ) || ( info->prg_size == 2 ) );

    assert( fread( prg_rom_0, sizeof( prg_rom_0 ), 1, file ) == 1 );
    if ( info->prg_size == 2 )
    {
        assert( fread( prg_rom_1, sizeof( prg_rom_1 ), 1, file ) == 1 );
    }

    printf( "\nTiNES init PRG, prg_blocks = %d.\n", info->prg_size );
    prg_size = info->prg_size;

    if ( !info->chr_size )
        return;
    fread( chr_rom, sizeof( chr_rom ), 1, file );
    printf( "TiNES init CHR, chr_blocks = %d.\n", info->chr_size );
    chr_size = info->chr_size;
}

void read_rom_info()
{
    printf( "%c %c", rom_hdr.prg_size_l, rom_hdr.rom_size_h );

    rom_info.prg_size = rom_hdr.rom_size_h & 0xF;
    rom_info.prg_size <<= 8;
    rom_info.prg_size += rom_hdr.prg_size_l;

    rom_info.chr_size = rom_hdr.rom_size_h & 0xF0;
    rom_info.chr_size <<= 4;
    rom_info.chr_size += rom_hdr.chr_size_l;

    // If the MSB nibble is $F, an exponent-multiplier notation is used:
    // The actual PRG-ROM map_size is 2^E *(MM*2+1) bytes.
    if ( rom_info.prg_size >= 0xF00 )
    {
        int e = ( rom_info.prg_size >> 2 ) & 0b111111;
        int m = rom_info.prg_size & 0b11;

        rom_info.prg_size = 1;
        while ( e-- )
            rom_info.prg_size *= 2;
        rom_info.prg_size *= ( m * 2 + 1 );
    }
    if ( rom_info.chr_size >= 0xF00 )
    {
        int e = ( rom_info.chr_size >> 2 ) & 0b111111;
        int m = rom_info.chr_size & 0b11;

        rom_info.chr_size = 1;
        while ( e-- )
            rom_info.chr_size *= 2;
        rom_info.chr_size *= ( m * 2 + 1 );
    }

    rom_info.mapper                = rom_hdr.flag6 >> 4;
    rom_info.nametable_mirror_type = rom_hdr.flag6 & 1;
}

void print_rom_info()
{
    printf( "\nTiNES emulator rom loaded:\n" );
    printf( "   PRG ROM = %d * 16KiB\n", rom_info.prg_size );
    printf( "   CHR ROM = %d * 8KB\n", rom_info.chr_size );
    printf( "MAPPER: %d\n", rom_info.mapper );
}
