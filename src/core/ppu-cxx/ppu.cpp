//
// Created by Yuhang Gu on 2023/8/30.
//

#include "ppu.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
// clang-format off
static const union tinus_palette_data {
    struct { uint8_t r, g, b, a; };
    uint32_t    data;
} tines_stdpalette[64] = {
    { 0x7F, 0x7F, 0x7F, 0xFF }, { 0x20, 0x00, 0xB0, 0xFF }, { 0x28, 0x00, 0xB8, 0xFF }, { 0x60, 0x10, 0xA0, 0xFF },
    { 0x98, 0x20, 0x78, 0xFF }, { 0xB0, 0x10, 0x30, 0xFF }, { 0xA0, 0x30, 0x00, 0xFF }, { 0x78, 0x40, 0x00, 0xFF },
    { 0x48, 0x58, 0x00, 0xFF }, { 0x38, 0x68, 0x00, 0xFF }, { 0x38, 0x6C, 0x00, 0xFF }, { 0x30, 0x60, 0x40, 0xFF },
    { 0x30, 0x50, 0x80, 0xFF }, { 0x00, 0x00, 0x00, 0xFF }, { 0x00, 0x00, 0x00, 0xFF }, { 0x00, 0x00, 0x00, 0xFF },

    { 0xBC, 0xBC, 0xBC, 0xFF }, { 0x40, 0x60, 0xF8, 0xFF }, { 0x40, 0x40, 0xFF, 0xFF }, { 0x90, 0x40, 0xF0, 0xFF },
    { 0xD8, 0x40, 0xC0, 0xFF }, { 0xD8, 0x40, 0x60, 0xFF }, { 0xE0, 0x50, 0x00, 0xFF }, { 0xC0, 0x70, 0x00, 0xFF },
    { 0x88, 0x88, 0x00, 0xFF }, { 0x50, 0xA0, 0x00, 0xFF }, { 0x48, 0xA8, 0x10, 0xFF }, { 0x48, 0xA0, 0x68, 0xFF },
    { 0x40, 0x90, 0xC0, 0xFF }, { 0x00, 0x00, 0x00, 0xFF }, { 0x00, 0x00, 0x00, 0xFF }, { 0x00, 0x00, 0x00, 0xFF },

    { 0xFF, 0xFF, 0xFF, 0xFF }, { 0x60, 0xA0, 0xFF, 0xFF }, { 0x50, 0x80, 0xFF, 0xFF }, { 0xA0, 0x70, 0xFF, 0xFF },
    { 0xF0, 0x60, 0xFF, 0xFF }, { 0xFF, 0x60, 0xB0, 0xFF }, { 0xFF, 0x78, 0x30, 0xFF }, { 0xFF, 0xA0, 0x00, 0xFF },
    { 0xE8, 0xD0, 0x20, 0xFF }, { 0x98, 0xE8, 0x00, 0xFF }, { 0x70, 0xF0, 0x40, 0xFF }, { 0x70, 0xE0, 0x90, 0xFF },
    { 0x60, 0xD0, 0xE0, 0xFF }, { 0x60, 0x60, 0x60, 0xFF }, { 0x00, 0x00, 0x00, 0xFF }, { 0x00, 0x00, 0x00, 0xFF },

    { 0xFF, 0xFF, 0xFF, 0xFF }, { 0x90, 0xD0, 0xFF, 0xFF }, { 0xA0, 0xB8, 0xFF, 0xFF }, { 0xC0, 0xB0, 0xFF, 0xFF },
    { 0xE0, 0xB0, 0xFF, 0xFF }, { 0xFF, 0xB8, 0xE8, 0xFF }, { 0xFF, 0xC8, 0xB8, 0xFF }, { 0xFF, 0xD8, 0xA0, 0xFF },
    { 0xFF, 0xF0, 0x90, 0xFF }, { 0xC8, 0xF0, 0x80, 0xFF }, { 0xA0, 0xF0, 0xA0, 0xFF }, { 0xA0, 0xFF, 0xC8, 0xFF },
    { 0xA0, 0xFF, 0xF0, 0xFF }, { 0xA0, 0xA0, 0xA0, 0xFF }, { 0x00, 0x00, 0x00, 0xFF }, { 0x00, 0x00, 0x00, 0xFF }
};

// clang-format on
ppu *ppu_inst;

ppu::ppu( uint8_t *chr_rom, int screen_arrangement )
    : scr_arrange( screen_arrangement )
{
    //    ppustatus = 0b10100000;
    memcpy( &pattern_tables.pattern_table_0, chr_rom, sizeof( pattern_tables.pattern_table_0 ) );
    memcpy( &pattern_tables.pattern_table_1, chr_rom + sizeof( pattern_tables.pattern_table_0 ), sizeof( pattern_tables.pattern_table_1 ) );
    //  ppu_inst = this;

    // mmap.emplace_back<mem_map_t>( { 0x0000, 0x1000, pattern_table_0 } );
    // mmap.emplace_back<mem_map_t>( { 0x1000, 0x1000, pattern_table_1 } );
    if ( screen_arrangement == HORIZON_SCREEN )
    {
        mmap.emplace_back<mem_map_t>( { 0x2000, 0x0400, ciram_0 } );
        mmap.emplace_back<mem_map_t>( { 0x2400, 0x0400, ciram_1 } );
        mmap.emplace_back<mem_map_t>( { 0x2800, 0x0400, ciram_0 } );
        mmap.emplace_back<mem_map_t>( { 0x2C00, 0x0400, ciram_1 } );
    }
    else if ( screen_arrangement == VERTICAL_SCREEN )
    {
        mmap.emplace_back<mem_map_t>( { 0x2000, 0x0400, ciram_0 } );
        mmap.emplace_back<mem_map_t>( { 0x2400, 0x0400, ciram_0 } );
        mmap.emplace_back<mem_map_t>( { 0x2800, 0x0400, ciram_1 } );
        mmap.emplace_back<mem_map_t>( { 0x2C00, 0x0400, ciram_1 } );
    }
    // mmap.emplace_back<mem_map_t>( { 0x3000, 0x0F00, nullptr, true, 0x2000 } );

    // $3F00 - Palette RAM
    palette_map[ 0 ] = &uni_bg_color;
    for ( int i = 1; i <= 0x0F; ++i )
    {
        palette_map[ i ] = bg_palette + i - 1;
    }
    //    palette_map[ 0x04 ] = &uni_bg_color;
    //    palette_map[ 0x08 ] = &uni_bg_color;
    //    palette_map[ 0x0C ] = &uni_bg_color;
    palette_map[ 0x10 ] = &uni_bg_color;
    palette_map[ 0x11 ] = sp_palette[ 0 ];
    palette_map[ 0x12 ] = sp_palette[ 0 ] + 1;
    palette_map[ 0x13 ] = sp_palette[ 0 ] + 2;
    palette_map[ 0x14 ] = palette_map[ 0x04 ];
    //    palette_map[ 0x14 ] = &uni_bg_color;
    palette_map[ 0x15 ] = sp_palette[ 1 ];
    palette_map[ 0x16 ] = sp_palette[ 1 ] + 1;
    palette_map[ 0x17 ] = sp_palette[ 1 ] + 2;
    palette_map[ 0x18 ] = palette_map[ 0x08 ];
    //    palette_map[ 0x18 ] = &uni_bg_color;
    palette_map[ 0x19 ] = sp_palette[ 2 ];
    palette_map[ 0x1A ] = sp_palette[ 2 ] + 1;
    palette_map[ 0x1B ] = sp_palette[ 2 ] + 2;
    palette_map[ 0x1C ] = palette_map[ 0x0C ];
    //    palette_map[ 0x1C ] = &uni_bg_color;
    palette_map[ 0x1D ] = sp_palette[ 3 ];
    palette_map[ 0x1E ] = sp_palette[ 3 ] + 1;
    palette_map[ 0x1F ] = sp_palette[ 3 ] + 2;

    init_io_register_handlers();
}

uint8_t &ppu::map_addr( uint16_t addr )
{
    addr %= 0x4000;
    if ( addr >= 0x3F00 ) // Pallete index
        return *palette_map[ addr % 0x20 ];
    else if ( addr >= 0x3000 ) // Mirrors
        return reinterpret_cast<uint8_t &>( *( pattern_tables.pattern_table_0 + addr - 0x1000 ) );
    if ( addr < 0x2000 ) // Pattern table
        return reinterpret_cast<uint8_t &>( *( pattern_tables.pattern_table_0 + addr ) );

    // nametable
    auto it = &mmap[ ( addr & 0x0F00 ) >> 0xA ];
    return *( it->map + ( addr - it->addr ) );
}
uint8_t ppu::mread( addr_t addr ) // 3F00 split search
{
    return map_addr( addr );
}

void ppu::mwrite( addr_t addr, byte data )
{
    map_addr( addr ) = data;
}

extern byte ppu_iobus_value;
void        ppu_reg_write( int idx, byte data )
{
    ppu_iobus_value = data;
    ppu_inst->ppu_io_reg[ idx % 8 ].write_handler( data );
}

byte ppu_reg_read( int idx )
{
    return ppu_inst->ppu_io_reg[ idx % 8 ].read_handler();
}

void oamdma_write( byte data )
{
}
byte oamdma_read()
{
    return 0;
}

uint32_t ppu::get_bg_palette_color( uint8_t index )
{
    if ( !index )
        return tines_stdpalette[ uni_bg_color ].data;
    if ( index >= 0x10 )
    {
        printf( "ERROR: GET BG COLOR OUT OF BOUND!\n" );
        //        assert( 0 );
    }
    index = ( index % 4 == 0 ) ? 0 : index;
    return tines_stdpalette[ *palette_map[ index ] ].data;
}

void ppu::render_bg( uint32_t *vmem )
{
    for ( int y = 0; y < 240; ++y )
    {
        for ( int x = 0; x < 256; ++x )
        {
            *( vmem + x + y * 256 ) = get_bg_color( x, y );
        }
    }
}
uint32_t ppu::get_bg_color( int x, int y )
{
    // FIXME: scroll, other palette
    int tile_x = x / 8;
    int tile_y = y / 8; // Nametable location
    int fine_x = x % 8;
    int fine_y = y % 8;

    // FIXME: scroll...
    addr_t bg_nt_base = 0x2000;

    addr_t pat_addr = mread( tile_y * 32 + tile_x + bg_nt_base ); // Pattern Table index
    pat_addr <<= 4;
    pat_addr |= fine_y; // Pattern Table row. TODO: another pat table for bg
                        //    pat_addr |= ( reinterpret_cast<ppuctrl_flag_t &>( ppuctrl ).bg_pattern_base ) ? 0x8 : 0;
    pat_addr += ( get_reg_data( PPUREG_CTRL ) & 0x10 ) ? 0x1000 : 0;
    // TODO: flip?
    uint8_t pattern_l = mread( pat_addr ) >> ( 7 - fine_x ) & 1;
    uint8_t pattern_h = mread( pat_addr + 8 ) >> ( 7 - fine_x ) & 1;
    uint8_t plte_idx  = ( pattern_h << 1 ) | pattern_l; // The Palette index for this pixel

    // Get the palette section in Attr. table
    int atr_x   = x / 32;
    int atr_y   = y / 32;
    fine_x      = x % 32 / 16;
    fine_y      = y % 32 / 16;
    int bit_off = fine_x * 2 + fine_y * 4; // Get Dx for palette index

    int atr_data     = mread( atr_y * 8 + atr_x + bg_nt_base + 0x3C0 );
    int plte_section = atr_data >> bit_off & 0b11;

    return get_bg_palette_color( plte_section * 4 + plte_idx );
}

bool is_ppu_nmi_enable()
{
    return reinterpret_cast<ppuctrl_flag_t &>( ppu_inst->get_reg_data( PPUREG_CTRL ) ).nmi_enable;
}
bool is_ppu_nmi_set()
{
    return reinterpret_cast<ppustatus_flag_t &>( ppu_inst->get_reg_data( PPUREG_STATUS ) ).nmi_flag;
}
byte get_vram_inc()
{
    return reinterpret_cast<ppuctrl_flag_t &>( ppu_inst->get_reg_data( PPUREG_CTRL ) ).vram_inc;
}

void set_ppu_nmi_enable( bool v )
{
    reinterpret_cast<ppuctrl_flag_t &>( ppu_inst->get_reg_data( PPUREG_CTRL ) ).nmi_enable = v;
}
void set_ppu_nmi( bool v )
{
    reinterpret_cast<ppustatus_flag_t &>( ppu_inst->get_reg_data( PPUREG_STATUS ) ).nmi_flag = v;
}

bool get_nmi_sig()
{
    static bool nmi_last;
    bool        nmi_set = is_ppu_nmi_set() && is_ppu_nmi_enable();
    bool        nmi_sig = !nmi_last && nmi_set;
    nmi_last            = nmi_set;
    return nmi_sig;
}