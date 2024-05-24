
cmake_minimum_required(VERSION 3.12)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

project(pico-sk6812
    VERSION 1.0.0
    DESCRIPTION "RP2040 library for controlling strips/pixels using SK6812 (RGBW) LEDs using PIO and pico-sdk"
)

add_library(pico-sk6812 INTERFACE)

target_sources(pico-sk6812 INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/sk6812.cpp
)

target_include_directories(pico-sk6812 INTERFACE ${CMAKE_CURRENT_LIST_DIR})

if(PICO_SDK)
    pico_generate_pio_header(pico-sk6812 ${CMAKE_CURRENT_LIST_DIR}/sk6812.pio)
endif(PICO_SDK)

# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(${PROJECT_NAME} INTERFACE
    pico_stdlib
    hardware_pio
)
