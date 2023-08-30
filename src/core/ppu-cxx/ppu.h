//
// Created by 顾雨杭 on 2023/8/30.
//

#ifndef EMULATOR_PPU_H
#define EMULATOR_PPU_H
#include <cassert>
#include <cstdint>
#include <vector>

#define addr_t uint16_t
#define byte uint8_t
struct mem_map_t
{
    addr_t addr;
    addr_t size;
    byte  *map;
    bool   enable_mirror;
    addr_t mirror_addr;
};

class ppu
{
  private:
    int scr_arrange;

    std::vector<mem_map_t> mmap;
    byte                  *palette_map[ 0x20 ] = {};
    addr_t                 vram_addr           = 0;
    bool                   vaddr_wr_h          = false;
    ////////////////////////////////////////////////////////////////////////////////////////
    // PPU Memory BUS. https://www.nesdev.org/wiki/PPU_memory_map
    byte pattern_table_0[ 0x1000 ] = {}; // Pattern table. $0000 - $0FFF
    byte pattern_table_1[ 0x1000 ] = {}; // Pattern table. $1000 - $1FFF
    byte ciram_0[ 0x0400 ]         = {}; // vram / nametable, $2000
    byte ciram_1[ 0x0400 ]         = {}; // $2400 (depend on the nametable mirror)

    byte uni_bg_color         = 0;
    byte bg_palette[ 15 ]     = {}; // $3F00 - $3F1F
    byte sp_palette[ 4 ][ 3 ] = {}; // $3F00 - $3F1F

    ////////////////////////////////////////////////////////////////////////////////////////
    // PPU Register Set. https://www.nesdev.org/wiki/PPU_registers
    byte ppuctrl   = 0; // $2000
    byte ppumask   = 0; // $2001
    byte ppustatus = 0; // $2002
    byte oamaddr   = 0; // $2003
    byte oamdata   = 0; // $2004
    byte ppuscroll = 0; // $2005
    byte ppuaddr   = 0; // $2006
    byte ppudata   = 0; // $2007
    byte oamdma    = 0; // $4014

  public:
    ppu( byte *chr_rom, int screen_arrangement );
    ~ppu() = default;

    ////////////////////////////////////////////////////////////////////////////////////////
    // PPU Memory
    byte  mread( addr_t addr );
    byte  mwrite( addr_t addr, byte data );
    byte &get_reg( int idx )
    {
        assert( idx >= 0 && idx <= 7 );
        return *( &ppuctrl + idx );
    }
    ////////////////////////////////////////////////////////////////////////////////////////
    // PPU Render
    void render_screen( uint32_t *vmem );

    friend void ppu_reg_write( int idx, byte data );
    friend byte ppu_reg_read( int idx );
};

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
//  C like interfaces
void ppu_reg_write( int idx, byte data );
byte ppu_reg_read( int idx );

enum
{
    HORIZON_SCREEN,
    VERTICAL_SCREEN,
};
enum ppu_reg_list
{
    PPUREG_CTRL,
    PPUREG_MASK,
    PPUREG_STATUS,
    PPUREG_OAMADDR,
    PPUREG_OAMDATA,
    PPUREG_SCROLL,

    PPUREG_ADDR, // 6
    PPUREG_DATA, // 7
};
//////////////////////////////////////////////////////////////////////////////////
// PPU Register reinterpret
ppu *ppu_inst;
struct ppuctrl_flag_t
{
    byte base_nametable : 2;
    byte vram_inc : 1;        //(0: add 1, going across; 1: add 32, going down)
    byte sp_pattern_base : 1; //(0: $0000; 1: $1000; ignored in 8x16 mode)
    byte bg_pattern_base : 1; //(0: $0000; 1: $1000; ignored in 8x16 mode)
    byte sp_size : 1;         // 0: 8x8;   1: 8x16
    byte ppu_ms_sel : 1;      // PPU master/slave select
    byte nmi_enable : 1;      // Generate an NMI at the start of the vertical blanking interval (0: off; 1: on)
};
struct ppustatus_flag_t // $2002
{
    byte ppu_open_bus : 5;
    byte sp_overflow : 1;
    byte sp_0_hit : 1;
    byte nmi_flag; // Vertical blank has started (0: not in vblank; 1: in vblank).
                   // Set at dot 1 of line 241 (the line *after* the post-render
                   // line); cleared after reading $2002 and at dot 1 of the
                   // pre-render line.
};

bool is_ppu_nmi_enable() { return reinterpret_cast<ppuctrl_flag_t &>( ppu_inst->get_reg( PPUREG_CTRL ) ).nmi_enable; }
bool is_ppu_nmi_set() { return reinterpret_cast<ppustatus_flag_t &>( ppu_inst->get_reg( PPUREG_STATUS ) ).nmi_flag; }
byte get_vram_inc() { return reinterpret_cast<ppuctrl_flag_t &>( ppu_inst->get_reg( PPUREG_CTRL ) ).vram_inc; }

void set_ppu_nmi_enable( bool v ) { reinterpret_cast<ppuctrl_flag_t &>( ppu_inst->get_reg( PPUREG_CTRL ) ).nmi_enable = v; }
void set_ppu_nmi( bool v ) { reinterpret_cast<ppustatus_flag_t &>( ppu_inst->get_reg( PPUREG_STATUS ) ).nmi_flag = v; }

#endif // EMULATOR_PPU_H
