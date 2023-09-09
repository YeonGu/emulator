///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/9/6
//
//  ppu-exec.cpp
//
///////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <ppu.h>

void cpu_call_nmi(); // in cpu_exec.cpp, call the NMI interrupt
void ppu::step( uint32_t *vmem )
{
    //    printf( "ppu step\n" );
    //    if ( frame_cnt == 50 )
    //        printf( "scanline=%d, cyc=%d\n", scanline, line_cycle );
    auto update_shifters = [ & ]() {
        if ( !is_render_bg() )
            return;
        bg_pattern_shift_l <<= 1;
        bg_pattern_shift_h <<= 1;
        bg_attribute_shift_l <<= 1;
        bg_attribute_shift_h <<= 1;
    };

    // =============================================================================
    // Increment the background tile "pointer" one tile/column horizontally.
    auto increment_scroll_x = [ & ]() {
        if ( !is_render_bg() )
            return;
        if ( ( vram_addr.data & 0x001F ) == 31 ) // if coarse X == 31
        {
            vram_addr.data &= ~0x001F; // coarse X = 0
            vram_addr.data ^= 0x0400;  // switch horizontal nametable
        }
        else
            vram_addr.data += 1; // increment coarse X
    };
    // =============================================================================
    // Increment y
    auto increment_y = [ & ]() {
        if ( !is_render_bg() )
            return;

        if ( ( vram_addr.data & 0x7000 ) != 0x7000 ) // if fine Y < 7
            vram_addr.data += 0x1000;                // increment fine Y
        else
        {
            vram_addr.data &= ~0x7000;                // fine Y = 0
            int y = ( vram_addr.data & 0x03E0 ) >> 5; // let y = coarse Y

            if ( y == 29 )
            {                             // bottom of the nametable (which is 32*30)
                y = 0;                    // coarse Y = 0
                vram_addr.data ^= 0x0800; // switch vertical nametable
            }
            else if ( y == 31 ) // coarse Y = 0, nametable not switched
                y = 0;
            else
                y += 1;                                                 // increment coarse Y
            vram_addr.data = ( vram_addr.data & ~0x03E0 ) | ( y << 5 ); // put coarse Y back into v
        }
    };
    auto get_tile_address = [ & ]() -> addr_t {
        return 0x2000 | ( vram_addr.data & 0x0FFF );
    };
    auto get_attribute_address = [ & ]() -> addr_t {
        addr_t addr = 0x23C0 | ( vram_addr.data & 0x0C00 ) | ( ( vram_addr.data >> 4 ) & 0x38 ) | ( ( vram_addr.data >> 2 ) & 0x07 );
        addr        = 0x2000 + 0x400 * vram_addr.nametable_x + 0x800 * vram_addr.nametable_y + 0x3C0 +
               ( ( vram_addr.coarse_y_h << 3 | vram_addr.coarse_y_l ) / 4 ) * 8 + ( vram_addr.coarse_x ) / 4;
        //        if ( is_render_bg() ) printf( "attr %4x -> %4x, scanline=%d, cycle=%d cx=%d cy=%d tile=%02x attr=%02x\n",
        //                                      get_tile_address(), addr, scanline, line_cycle, vram_addr.coarse_x, vram_addr.coarse_y_h << 3 | vram_addr.coarse_y_l, mread( get_tile_address() ), mread( addr ) );
        return addr;
        //        return 0x23C0 | ( vram_addr.nametable_y << 11 ) | ( vram_addr.nametable_x << 10 ) | ( ( ( vram_addr.coarse_y_l ) | ( vram_addr.coarse_y_h << 3 ) >> 2 ) << 3 ) | ( vram_addr.coarse_x >> 2 );
    };

    // Load pattern data (h & l)
    // vram_addr -> nametable data (pattern index) -> pattern data
    auto load_pattern_data = [ & ]() {
        // Pattern table index in nametable
        auto   id       = mread( get_tile_address() );
        addr_t pat_addr = (addr_t) id << 4;
        pat_addr |= ( get_reg_data( PPUREG_CTRL ) & 0x10 ) ? 0x1000 : 0;
        pat_addr |= vram_addr.fine_y;

        bg_next_pattern_l = mread( pat_addr );
        bg_next_pattern_h = mread( pat_addr + 8 );
    };

    // Load Attribute data
    // vram_addr -> attribute data -> bg_next_attribute latch
    auto load_attribute_data = [ & ]() {
        bg_next_attribute = mread( get_attribute_address() );
        if ( vram_addr.coarse_x & 0x02 ) bg_next_attribute >>= 2;
        if ( vram_addr.coarse_y_l & 0x02 ) bg_next_attribute >>= 4;
        bg_next_attribute &= 0x03;

        //        if ( is_render_bg() )
        //            printf( "   next palette index=%x\n", bg_next_attribute );
    };

    auto load_bg_shifters = [ & ]() {
        bg_pattern_shift_l &= 0xFF00;
        bg_pattern_shift_h &= 0xFF00;
        bg_attribute_shift_l &= 0xFF00;
        bg_attribute_shift_h &= 0xFF00;

        bg_pattern_shift_l |= bg_next_pattern_l;
        bg_pattern_shift_h |= bg_next_pattern_h;
        bg_attribute_shift_l |= ( bg_next_attribute & 0x01 ) ? 0xFF : 0;
        bg_attribute_shift_h |= ( bg_next_attribute & 0x02 ) ? 0xFF : 0;
    };
    auto reset_vram_horizon = [ & ]() {
        if ( !is_render_bg() )
            return;
        vram_addr.coarse_x    = tmp_addr.coarse_x;
        vram_addr.nametable_x = tmp_addr.nametable_x;
    };
    auto reset_vram_vertical = [ & ]() {
        if ( !is_render_bg() )
            return;
        vram_addr.fine_y      = tmp_addr.fine_y;
        vram_addr.nametable_y = tmp_addr.nametable_y;
        vram_addr.coarse_y_l  = tmp_addr.coarse_y_l;
        vram_addr.coarse_y_h  = tmp_addr.coarse_y_h;
    };

    // During Pre-render and Visible scanlines
    if ( scanline >= -1 && scanline <= 239 )
    {
        //        printf( "scanline=%d\n", scanline );
        // Skip one cycle when BG+0+odd
        if ( scanline == 0 && line_cycle == 0 && ( frame_cnt % 2 ) )
        {
            line_cycle = 1;
        }
        // Clear VBlank Flag
        if ( scanline == -1 && line_cycle == 1 )
        {
            reinterpret_cast<ppustatus_flag_t &>( get_reg_data( PPUREG_STATUS ) ).nmi_flag = 0;
        }

        // PPU render loop.
        // The cycle 0 does no operations to registers.
        // So the register operation happens from cycle 2. (pixel 2)
        if ( ( line_cycle >= 2 && line_cycle < 258 ) || ( line_cycle >= 321 && line_cycle < 338 ) )
        {
            update_shifters();

            switch ( ( line_cycle - 1 ) % 8 )
            {
            case 1:
                load_pattern_data();
                break;
            case 2:
                load_attribute_data();
                break;
            case 7:
                increment_scroll_x();
                break;
            case 0:
                load_bg_shifters();
                break;
            default:
                break;
            }
        }

        if ( line_cycle == 257 )
        {
            increment_y();
            reset_vram_horizon();
            load_bg_shifters();
        }

        // During the Pre-render scanline,Reset vertical address during [280, 304]
        if ( ( scanline == -1 ) && ( line_cycle >= 280 ) && ( line_cycle <= 304 ) )
            reset_vram_vertical();
    }
    if ( scanline == 241 && line_cycle == 1 )
    {
        set_ppu_nmi( true );
        if ( is_ppu_nmi_enable() )
            cpu_call_nmi();
    }
    // Render!
    if ( ( scanline >= 0 && scanline <= 239 ) && ( line_cycle <= 256 ) && line_cycle > 0 )
    {
        uint8_t pattern         = ( bg_pattern_shift_h >> 14 & 0x2 ) | ( bg_pattern_shift_l >> 15 );
        uint8_t palette_section = ( bg_attribute_shift_h >> 14 & 0x2 ) | ( bg_attribute_shift_l >> 15 );

        int  offset    = scanline * 256 + line_cycle - 1;
        auto col       = get_bg_palette_color( palette_section * 4 + pattern );
        vmem[ offset ] = col;
    }

    scanline += ( line_cycle == 340 ) ? 1 : 0;
    scanline   = ( scanline > 260 ) ? -1 : scanline;
    line_cycle = ( line_cycle == 340 ) ? 0 : line_cycle + 1;
    if ( scanline == -1 && line_cycle == 0 )
        frame_cnt++;

    //    if ( ( scanline == -1 ) && ( line_cycle == 339 ) && ( frame_cnt % 2 ) ) // skip the last pixel in odd frame
    //        line_cycle++;
    //    else if ( scanline >= 0 && scanline <= 239 ) // Visible-render
    //    {
    //        if ( !line_cycle )
    //            int a = 0;
    //        else if ( line_cycle <= 256 )
    //        {
    //            int offset     = scanline * 256 + line_cycle - 1;
    //            vmem[ offset ] = get_bg_color( line_cycle - 1, scanline );
    //        }
    //    }
    //    else if ( scanline == 241 && line_cycle == 1 )
    //    {
    //        reinterpret_cast<ppustatus_flag_t &>( get_reg_data( PPUREG_STATUS ) ).nmi_flag = 1;
    //        if ( reinterpret_cast<ppuctrl_flag_t &>( get_reg_data( PPUREG_CTRL ) ).nmi_enable )
    //        { // During Frame 0, 1, NMI is disabled.
    //            cpu_call_nmi();
    //        }
    //    }
    //    else if ( scanline == 260 && line_cycle == 332 ) // The VBL flag ($2002.7) is cleared by the PPU around 2270 CPU clocks
    //    {                                                //        after NMI occurs. (vbl clear time test)
    //        reinterpret_cast<ppustatus_flag_t &>( get_reg_data( PPUREG_STATUS ) ).nmi_flag = 0;
    //    }
    //
    //    // status update
    //    if ( line_cycle == 340 )
    //        scanline = ( scanline == 260 ) ? -1 : scanline + 1;
    //    if ( line_cycle == 340 && ( scanline == 260 ) )
    //        frame_cnt++;
    //    line_cycle = ( line_cycle == 340 ) ? 0 : line_cycle + 1;
}