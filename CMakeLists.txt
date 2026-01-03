# Copyright 2025-2026 Roos Catling-Tate
#
# Permission to use, copy, modify, and/or distribute this software for any purpose with or
# without fee is hereby granted, provided that the above copyright notice and this permission
# notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED “AS IS” AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
# IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_COMPILER_FORCED True)
set(CMAKE_C_COMPILER_FORCED True)

if (USE_SPIDEV)
    if (USE_ARM_CC)
        include("cmake/linux-arm-cross-compile.cmake")
    elseif (USE_ARMV7_CC)
        include("cmake/linux-armv7-cross-compile.cmake")
    else ()
        include("cmake/linux-aarch64-cross-compile.cmake")
    endif ()
elseif (NOT USE_PRINT_DEBUG) # USE_PICO_LIB as default, for USE_PRINT_DEBUG just skip
    include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
endif ()

project(piMCP2515 C CXX ASM)

if (USE_PICO_LIB)
    set(PICO_BOARD pico)
    pico_sdk_init()
endif ()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_library(piMCP2515 STATIC src/pi_MCP2515.c src/gpio.c src/pi_MCP2515.h src/gpio.h)

if (USE_SPIDEV)
    target_compile_definitions(piMCP2515 PRIVATE USE_SPIDEV=1)
elseif (USE_PRINT_DEBUG)
    target_compile_definitions(piMCP2515 PRIVATE USE_PRINT_DEBUG=1)
else () # USE_PICO_LIB
    target_compile_definitions(piMCP2515 PRIVATE USE_PICO_LIB=1)

    target_link_libraries(${PROJECT_NAME}
            pico_stdlib
            hardware_spi
    )
endif ()

unset(USE_PICO_LIB CACHE)
unset(USE_SPIDEV CACHE)
unset(USE_PRINT_DEBUG CACHE)
