///////////////////////////////////////////////////////////////////////
//          TiNES FC/NES Emulator
//          Southeast University.
//  Created by Gu Yuhang on 2023/9/6
//
//  ppu-exec.cpp
//
///////////////////////////////////////////////////////////////////////

#include <ppu.h>

void cpu_call_nmi(); // in cpu_exec.cpp, call the NMI interrupt
void ppu::step( uint32_t *vmem )
{
    // =============================================================================
    // Increment the background tile "pointer" one tile/column horizontally.
    auto increment_coarse_x = [ & ]() {
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
        return 0x23C0 | ( vram_addr.data & 0x0C00 ) | ( ( vram_addr.data >> 4 ) & 0x38 ) | ( ( vram_addr.data >> 2 ) & 0x07 );
    };

    if ( ( scanline == -1 ) && ( line_cycle == 339 ) && ( frame_cnt % 2 ) ) // skip the last pixel in odd frame
        line_cycle++;
    else if ( scanline >= 0 && scanline <= 239 ) // Visible-render
    {
        if ( !line_cycle )
            int a = 0;
        else if ( line_cycle <= 256 )
        {
            int offset     = scanline * 256 + line_cycle - 1;
            vmem[ offset ] = get_bg_color( line_cycle - 1, scanline );
        }
    }
    else if ( scanline == 241 && line_cycle == 1 )
    {
        reinterpret_cast<ppustatus_flag_t &>( get_reg_data( PPUREG_STATUS ) ).nmi_flag = 1;
        if ( reinterpret_cast<ppuctrl_flag_t &>( get_reg_data( PPUREG_CTRL ) ).nmi_enable )
        { // During Frame 0, 1, NMI is disabled.
            cpu_call_nmi();
        }
    }
    else if ( scanline == 260 && line_cycle == 332 ) // The VBL flag ($2002.7) is cleared by the PPU around 2270 CPU clocks
    {                                                //        after NMI occurs. (vbl clear time test)
        reinterpret_cast<ppustatus_flag_t &>( get_reg_data( PPUREG_STATUS ) ).nmi_flag = 0;
    }

    // status update
    if ( line_cycle == 340 )
        scanline = ( scanline == 260 ) ? -1 : scanline + 1;
    if ( line_cycle == 340 && ( scanline == 260 ) )
        frame_cnt++;
    line_cycle = ( line_cycle == 340 ) ? 0 : line_cycle + 1;
}