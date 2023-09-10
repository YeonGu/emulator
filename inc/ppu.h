//
// Created by Yuhang Gu on 2023/8/30.
//

#ifndef EMULATOR_PPU_H
#define EMULATOR_PPU_H
#include <cassert>
#include <cstdint>
#include <functional>
#include <vector>
using addr_t = uint16_t;
using byte   = uint8_t;
struct mem_map_t
{
    addr_t addr;
    addr_t size;
    byte  *map;
};

class ppu
{
  public:
    byte mread( addr_t addr );
    void mwrite( addr_t addr, byte data );

  private:
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
    int scr_arrange;
    ////////////////////////////////////////////////////////////////////////////////////////
    /// PPU status. see: PPU rendering
    int      scanline   = -1;
    int      line_cycle = 0;
    uint64_t frame_cnt  = 0;

    ////////////////////////////////////////////////////////////////////////////////////////
    // PPU internal regs. see: PPU scroll
    union vram_addr_t {
        uint16_t data;
        struct
        {
            uint8_t coarse_x : 5;
            uint8_t coarse_y_l : 3;
            uint8_t coarse_y_h : 2;
            uint8_t nametable_x : 1;
            uint8_t nametable_y : 1;
            uint8_t fine_y : 3;
            uint8_t unused : 1;
        } __attribute__( ( packed ) );
    };
    vram_addr_t vram_addr;
    vram_addr_t tmp_addr;
    byte        finex_scroll = 0;
    bool        write_toggle = false;

    ////////////////////////////////////////////////////////////////////////////////////////
    // PPU Register Set. https://www.nesdev.org/wiki/PPU_registers
    using reg_read_behavior  = std::function<byte()>;
    using reg_write_behavior = std::function<void( byte )>;
    void init_io_register_handlers();
    struct ppu_io_register_t
    {
        byte              reg_data     = 0;
        reg_read_behavior read_handler = [ this ]() {
            return reg_data;
        };
        reg_write_behavior write_handler = [ this ]( byte data ) {
            reg_data = data;
        };
    } ppu_io_reg[ 8 ];

  private:
    using oam_addr_t = uint8_t;
    struct ppu_oam_t
    {
        uint8_t y;
        uint8_t tile_id;
        uint8_t attributes;
        uint8_t x;
    } oam[ 64 ];
    oam_addr_t *oam_entry = (oam_addr_t *) oam;
    oam_addr_t  oam_addr;

    ppu_oam_t sprite_scanline[ 8 ];
    uint8_t   sprite_cnt = 0;
    uint8_t   sp_pattern_shifter_lo[ 8 ];
    uint8_t   sp_pattern_shifter_hi[ 8 ];

    bool sp0_possible_rendered;
    bool sp0_being_rendered;

    // =====================================================================================
    // PPU Register Translation
    struct ppu_mask_reg_t
    {
        uint8_t grayscale : 1;
        uint8_t show_left_8_pix_bg : 1;
        uint8_t show_left_8_pix_sp : 1;
        uint8_t show_bg : 1;
        uint8_t show_sp : 1;
        uint8_t empasize_red : 1;
        uint8_t empasize_green : 1;
        uint8_t empasize_blue : 1;
    };
    bool is_render_bg()
    {
        return reinterpret_cast<ppu_mask_reg_t &>( ppu_io_reg[ PPUREG_MASK ] ).show_bg;
    };
    bool is_render_sp()
    {
        return reinterpret_cast<ppu_mask_reg_t &>( ppu_io_reg[ PPUREG_MASK ] ).show_sp;
    };

    uint16_t bg_pattern_shift_l;
    uint16_t bg_pattern_shift_h;
    uint16_t bg_attribute_shift_l;
    uint16_t bg_attribute_shift_h;

    uint8_t bg_next_pattern_l;
    uint8_t bg_next_pattern_h;
    uint8_t bg_next_attribute;

    ////////////////////////////////////////////////////////////////////////////////////////
    // PPU Memory. https://www.nesdev.org/wiki/PPU_memory_map
    // TODO: refactor this shit
    std::vector<mem_map_t> mmap;
    byte                  *palette_map[ 0x20 ] = {};
    struct
    {
        byte pattern_table_0[ 0x1000 ]; // Pattern table. $0000 - $0FFF
        byte pattern_table_1[ 0x1000 ]; // Pattern table. $1000 - $1FFF
    } __attribute__( ( packed ) ) pattern_tables;
    byte ciram_0[ 0x0400 ] = {}; // vram / nametable, $2000
    byte ciram_1[ 0x0400 ] = {}; // $2400 (depend on the nametable mirror)

    byte uni_bg_color         = 0;
    byte bg_palette[ 15 ]     = {}; // $3F00 - $3F1F
    byte sp_palette[ 4 ][ 3 ] = {}; // $3F00 - $3F1F

    byte &map_addr( addr_t addr );

    uint32_t get_bg_palette_color( uint8_t index );
    uint32_t get_bg_color( int x, int y );
    uint32_t get_fg_palette_color( uint8_t index );

  public:
    ppu( byte *chr_rom, int screen_arrangement );
    ~ppu() = default;

    ////////////////////////////////////////////////////////////////////////////////////////
    // PPU Step
    void step( uint32_t *vmem );

    ////////////////////////////////////////////////////////////////////////////////////////
    // PPU Render
    void render_bg( uint32_t *vmem );

    byte &get_reg_data( int idx )
    {
        assert( idx >= 0 && idx <= 7 );
        return ppu_io_reg[ idx ].reg_data;
    }
    friend void ppu_reg_write( int idx, byte data );
    friend byte ppu_reg_read( int idx );
    friend void test_loop();
    friend void oamdma_write( byte dma_page );
};

extern ppu *ppu_inst;
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
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
//  C like interfaces
void ppu_reg_write( int idx, byte data );
byte ppu_reg_read( int idx );
void oamdma_write( byte dma_page );
byte oamdma_read();

enum
{
    HORIZON_SCREEN,
    VERTICAL_SCREEN,
};

//////////////////////////////////////////////////////////////////////////////////
// PPU Register reinterpret
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
    byte nmi_flag : 1; // Vertical blank has started (0: not in vblank; 1: in vblank).
                       // Set at dot 1 of line 241 (the line *after* the post-render
                       // line); cleared after reading $2002 and at dot 1 of the
                       // pre-render line.
};

bool is_ppu_nmi_enable();
bool is_ppu_nmi_set();
byte get_vram_inc();
void set_ppu_nmi_enable( bool v );
void set_ppu_nmi( bool v );

#endif // EMULATOR_PPU_H
