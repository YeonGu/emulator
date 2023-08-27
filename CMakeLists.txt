cmake_minimum_required(VERSION 3.24)
project(emulator C)

set(CMAKE_C_STANDARD 11)

include_directories(inc)

add_executable(emulator
        inc/rom.h
        src/main.c
        src/nes-rom.c
        src/memory.c
        inc/memory.h
        src/cpu.c
        inc/cpu.h
        src/cpu_exec.c
        inc/configs.h
        inc/cpu_datas.h
        src/mapper.c
        inc/ppu-reg.h
        src/nes.c
        src/nes.c
        inc/nes.h)
