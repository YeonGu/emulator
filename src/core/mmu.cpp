//
// Created by Zinan Xie on 2023/9/1.
//

#include "mmu.h"

mmu<uint16_t, uint8_t> *vmmu = nullptr;

mmu<uint16_t, uint8_t> *get_mmu()
{
    if ( vmmu == nullptr )
        vmmu = new mmu<uint16_t, uint8_t>( new uint8_t[ 0x10000 ], sizeof( uint8_t[ 0x10000 ] ) );
    return vmmu;
}
