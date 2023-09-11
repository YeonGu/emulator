//
// Created by Zinan Xie on 2023/9/1.
//

#include "mmu.h"

mmu<uint16_t, uint8_t> *vmmu = nullptr;

void init_mmu()
{
    delete[] vmmu;
    vmmu = new mmu<uint16_t, uint8_t>(sizeof( uint8_t[ 0x10000 ] ) );
}

mmu<uint16_t, uint8_t> *get_mmu()
{
    return vmmu;
}
