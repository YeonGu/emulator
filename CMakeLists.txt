cmake_minimum_required(VERSION 3.24)
project(emulator)

set(CMAKE_C_STANDARD 11)

set(SDL2_LIB_DIR D:/Library/clibs/SDL2-2.28.2/x86_64-w64-mingw32)

include_directories(inc)
include_directories(${SDL2_LIB_DIR}/include)

add_compile_options(-lSDL2 -lm -ldl)

add_executable(emulator
        inc/rom.h
        src/main.cpp
        src/nes-rom.cpp
        src/core/memory.cpp
        inc/memory.h
        src/core/cpu.cpp
        inc/cpu.h
        src/core/cpu_exec.cpp
        inc/configs.h
        inc/cpu_datas.h
        src/core/mapper.cpp
        #        inc/ppu-reg.h
        src/nes.cpp
        src/nes.cpp
        inc/nes.h
        #        src/core/ppu/ppu.cpp
        #        inc/ppu.h
        #        src/core/ppu/ppu-reg-interface.cpp
        #        inc/ppu-memory.h
        #        src/core/ppu/ppu-memory.cpp
        #        src/core/ppu/ppu-palette.cpp
        #        src/core/ppu/render.cpp
        src/core/ppu-cxx/ppu.cpp
        inc/ppu.h)

#target_link_libraries(emulator ${SDL2_LIB_DIR}/lib/libSDL2.dylib ${SDL2_LIB_DIR}/lib/libSDL2main.a ${SDL2_LIB_DIR}/lib/libSDL2.a)
target_link_libraries(emulator ${SDL2_LIB_DIR}/lib/libSDL2.dll.a ${SDL2_LIB_DIR}/lib/libSDL2main.a ${SDL2_LIB_DIR}/lib/libSDL2.a)