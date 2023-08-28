//
// Created by 顾雨杭 on 2023/8/25.
//

#ifndef EMULATOR_CPU_DATAS_H
#define EMULATOR_CPU_DATAS_H

#include "stdint.h"

uint8_t inst_base_cycles[ 256 ] = {
    // # 0x00-0x0F
    7, 6, 0, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6,
    // # 0x10-0x1F
    2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    // # 0x20-0x2F
    6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 4, 2, 4, 4, 6, 6,
    // # 0x30-0x3F
    2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    // # 0x40-0x4F
    6, 6, 0, 8, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6,
    // # 0x50-0x5F
    2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    // # 0x60-0x6F
    6, 6, 0, 8, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6,
    // # 0x70-0x7F
    2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    // # 0x80-0x8F
    2, 6, 2, 0, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
    // # 0x90-0x9F
    2, 6, 0, 0, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5,
    // # 0xA0-0xAF
    2, 6, 2, 0, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
    // # 0xB0-0xBF
    2, 5, 0, 0, 4, 4, 4, 4, 2, 4, 2, 7, 4, 4, 7, 7,
    // # 0xC0-0xCF
    2, 6, 2, 0, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
    // # 0xD0-0xDF
    2, 5, 0, 0, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    // # 0xE0-0xEF
    2, 6, 0, 0, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
    // # 0xF0-0xFF
    2, 5, 0, 0, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7 };

#endif // EMULATOR_CPU_DATAS_H