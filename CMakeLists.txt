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

if (USE_SPI)
    if (USE_ARM_LINUX_CC)
        include("cmake/linux-arm-cross-compile.cmake")
    elseif (USE_AARCH64_LINUX_CC)
        include("cmake/linux-aarch64-cross-compile.cmake")
    elseif (USE_AARCH64_NETBSD_CC)
        include("cmake/netbsd-aarch64-cross-compile.cmake")
    elseif (USE_AARCH64_FREEBSD_CC)
        include("cmake/freebsd-aarch64-cross-compile.cmake")
    endif ()
    project(piMCP2515 C)
else ()
    include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
    set(USE_PICO_LIB 1)
    project(piMCP2515 C CXX ASM)
    set(CMAKE_C_STANDARD 99)
endif ()

include(cmake/doxygen.cmake)

if (USE_PICO_LIB)
    set(PICO_BOARD pico)
    pico_sdk_init()
    set(CMAKE_C_STANDARD 11)
    set(CMAKE_CXX_STANDARD 17)
endif ()

set(LIB_SOURCES src/pi_MCP2515.c src/pi_MCP2515.h
        src/gpio.c src/gpio.h
        src/can.c src/can.h
        src/status_error.c src/status_error.h
        src/interrupt.c src/interrupt.h
        src/filter.c src/filter.h
        src/reqop.c src/reqop.h
        src/registers.c src/registers.h
        src/debug.c src/debug.h
        src/time.c src/time.h
        src/pi_MCP2515_handle.h
        include/pi_MCP2515_defs.h)

add_library(piMCP2515_objects OBJECT ${LIB_SOURCES})
set_target_properties(piMCP2515_objects PROPERTIES POSITION_INDEPENDENT_CODE ON)
add_library(piMCP2515_static STATIC $<TARGET_OBJECTS:piMCP2515_objects>)
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

if (USE_PICO_LIB)
    target_compile_definitions(piMCP2515_objects PRIVATE USE_PICO_LIB=1)
    set_target_properties(piMCP2515_static PROPERTIES OUTPUT_NAME "piMCP2515_pico")
    target_link_libraries(piMCP2515_static pico_stdlib hardware_spi)
else ()
    set_target_properties(piMCP2515_static PROPERTIES OUTPUT_NAME "piMCP2515")
    add_library(piMCP2515_shared SHARED $<TARGET_OBJECTS:piMCP2515_objects>)
    set_target_properties(piMCP2515_shared PROPERTIES OUTPUT_NAME "piMCP2515")
    target_compile_definitions(piMCP2515_objects PRIVATE USE_SPI=1)
    install(TARGETS piMCP2515_shared LIBRARY DESTINATION lib)
    install(TARGETS piMCP2515_static ARCHIVE DESTINATION lib)
    install(FILES include/pi_MCP2515.h include/pi_MCP2515_defs.h DESTINATION include)
endif ()

unset(USE_PICO_LIB CACHE)
unset(USE_SPI CACHE)
unset(USE_AARCH64_LINUX_CC CACHE)
unset(USE_ARM_LINUX_CC CACHE)
unset(USE_AARCH64_FREEBSD_CC CACHE)
