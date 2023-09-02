//
// Created by 谢子南 on 2023/9/1.
//

#ifndef EMULATOR_MMU_H
#define EMULATOR_MMU_H

#include <cstdint>
#include <functional>

template <typename addr_t, typename data_t>
class mmu
{
  public:
    using read_behaviour  = std::function<data_t( addr_t vaddr )>;
    using write_behaviour = std::function<void( addr_t vaddr, data_t data )>;

    struct iterator
    {
        addr_t vaddr;
        mmu   *_mmu;

        operator data_t()
        {
            return _mmu->m_read( vaddr );
        }
        iterator &operator=( data_t _data )
        {
            _mmu->m_write( vaddr, _data );
            return *this;
        }
    };

  private:
    struct mmap_t
    {
        void*          addr;
        read_behaviour  _read_b;
        write_behaviour _write_b;
    }      *_mmap;

  public:
    mmu( size_t vaddr_max_size )
    {
        _mmap     = new mmap_t[ vaddr_max_size ];
    }

    void* vaddr_to_addr( addr_t vaddr )
    {
        return _mmap[ vaddr ].addr;
    }

    void mmap( addr_t vaddr, const void* addr, size_t size )
    {
        for ( size_t offset = 0; offset < size; offset++ )
        {
            _mmap[ vaddr + offset ].addr    = (char*)addr + offset;
            _mmap[ vaddr + offset ]._read_b = [ & ]( addr_t _vaddr ) -> data_t {
                return  *(data_t *)this->vaddr_to_addr( _vaddr );
            };
            _mmap[ vaddr + offset ]._write_b = [ & ]( addr_t _vaddr, data_t _data ) -> void {
                *(data_t *)this->vaddr_to_addr( vaddr ) = _data;
            };
        }
    }

    void mmap( addr_t vaddr, const void* addr, size_t size,
               read_behaviour  read_b,
               write_behaviour write_b )
    {
        for ( size_t offset = 0; offset < size; offset++ )
        {
            _mmap[ vaddr + offset ].addr     = (char*)addr + offset;
            _mmap[ vaddr + offset ]._read_b  = read_b;
            _mmap[ vaddr + offset ]._write_b = write_b;
        }
    }

    void m_write( addr_t vaddr, data_t data )
    {
        _mmap[ vaddr ]._write_b( vaddr, data );
    }

    data_t m_read( addr_t vaddr )
    {
        return _mmap[ vaddr ]._read_b( vaddr );
    }

    iterator operator[]( addr_t vaddr )
    {
        return iterator{ vaddr, this };
    }
};

mmu<uint16_t, uint8_t> *get_mmu();

#endif // EMULATOR_MMU_H
