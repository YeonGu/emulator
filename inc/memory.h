//
// Created by Gu Yuhang on 2023/8/21.
//

#ifndef EMULATOR_MEMORY_H
#define EMULATOR_MEMORY_H
#include "rom.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#define NMI_VECTOR_ADDR 0xFFFA
#define RESET_VECTOR_ADDR 0xFFFC
#define IRQ_BRK_VECTOR_ADDR 0xFFFE

#define addr_t uint16_t

struct cpu_mem_map_t
{
    char    *map_name;
    uint8_t *map_begin;
    addr_t   nes_begin;
    uint16_t map_size;

    void ( *mem_write_handler )( addr_t offset, uint8_t data );
    void ( *mem_read_handler )( addr_t offset );
};

void fetch_rom( FILE *file, struct nes_rom_info_t *info );
void init_mapper( struct nes_rom_info_t *info );

uint8_t vaddr_read( addr_t addr );
void    vaddr_write( addr_t addr, uint8_t data );

#endif // EMULATOR_MEMORY_H
