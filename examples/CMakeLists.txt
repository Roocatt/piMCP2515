include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(piMCP2515-examples C CXX ASM)

set(PICO_BOARD pico)
pico_sdk_init()
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

function(add_example name)
    add_executable(piMCP2515-example-${name} ${name}.c)
    target_include_directories(piMCP2515-example-${name} PRIVATE ../include)
    target_link_directories(piMCP2515-example-${name} PRIVATE ../)
    target_link_libraries(piMCP2515-example-${name} pico_stdlib hardware_spi piMCP2515_pico)
endfunction()

add_example(01_init_and_read_loop)
add_example(02_read_specific_buffer_loop)
add_example(03_send_messages)