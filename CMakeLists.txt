cmake_minimum_required(VERSION 3.24)
project(emulator C)

set(CMAKE_C_STANDARD 11)

set(SDL2_LIB_DIR D:/Library/clibs/SDL2-2.28.2/x86_64-w64-mingw32)

include_directories(inc)
include_directories(${SDL2_LIB_DIR}/include)

add_executable(emulator
        inc/rom.h
        src/main.c
        src/nes-rom.c
        src/core/memory.c
        inc/memory.h
        src/core/cpu.c
        inc/cpu.h
        src/core/cpu_exec.c
        inc/configs.h
        inc/cpu_datas.h
        src/core/mapper.c
        inc/ppu-reg.h
        src/nes.c
        src/nes.c
        inc/nes.h
        src/core/ppu/ppu.c
        inc/ppu.h
        src/core/ppu/ppu-reg-interface.c
        inc/ppu-memory.h
        src/core/ppu/ppu-memory.c
        src/core/ppu/ppu-palette.c)

target_link_libraries(emulator ${SDL2_LIB_DIR}/lib/libSDL2.dll.a ${SDL2_LIB_DIR}/lib/libSDL2main.a ${SDL2_LIB_DIR}/lib/libSDL2.a)