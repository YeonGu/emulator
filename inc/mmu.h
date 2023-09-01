//
// Created by 谢子南 on 2023/9/1.
//

#ifndef EMULATOR_MMU_H
#define EMULATOR_MMU_H

#include <functional>

template<typename addr_t,typename data_t>
class mmu {
public:
    using read_behaviour = std::function<data_t&(addr_t vaddr)>;
    using write_behaviour = std::function<void(addr_t vaddr,data_t data)>;

    struct iterator
    {
        addr_t vaddr;
        mmu* _mmu;

        operator data_t()
        {
            return _mmu->m_read(vaddr);
        }
        iterator& operator=(data_t _data)
        {
            _mmu->m_write(vaddr,_data);
            return *this;
        }
    };

private:
    struct mmap_t
    {
        addr_t addr;
        read_behaviour _read_b;
        write_behaviour _write_b;
    }*_mmap;
    iterator* iterator_val;
    data_t* mem;
public:

    mmu(data_t* mem,addr_t mem_size) {
        _mmap = new mmap_t[mem_size];
        this->mem = mem;
    }

    addr_t vaddr_to_addr(addr_t vaddr)
    {
        return _mmap[vaddr].addr;
    }

    void mmap(addr_t vaddr,addr_t addr,addr_t size)
    {
        for (addr_t offset = 0;offset < size;offset++)
        {
            _mmap[vaddr + offset].addr = addr + offset;
            _mmap[vaddr + offset]._read_b = [&](addr_t _vaddr) -> data_t&{
                return reinterpret_cast<data_t&>(mem[this->vaddr_to_addr(_vaddr)]);
            };
            _mmap[vaddr + offset]._write_b = [&](addr_t _vaddr,data_t _data) -> void
            {
                mem[this->vaddr_to_addr(vaddr)] = _data;
            };
        }
    }

    void mmap(addr_t vaddr,addr_t addr, addr_t size,
              read_behaviour read_b,
              write_behaviour write_b)
    {
        for (addr_t offset = 0;offset < size;offset++)
        {
            _mmap[vaddr + offset].addr = addr + offset;
            _mmap[vaddr + offset]._read_b = read_b;
            _mmap[vaddr + offset]._write_b = write_b;
        }
    }

    void m_write(addr_t vaddr,data_t data)
    {
        _mmap[vaddr]._write_b(vaddr,data);
    }

    data_t& m_read(addr_t vaddr)
    {
        return _mmap[vaddr]._read_b(vaddr);
    }

    iterator operator[](addr_t vaddr)
    {
        return iterator{vaddr,this};
    }

};

template<typename addr_t,typename data_t>
mmu<addr_t,data_t>* get_mmu();

#endif //EMULATOR_MMU_H
