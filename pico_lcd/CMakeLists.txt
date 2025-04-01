# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)
include(FetchContent)

project(pico_lcd C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(pico_lcd pico_lcd.cpp )

pico_set_program_name(pico_lcd "pico_lcd")
pico_set_program_version(pico_lcd "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(pico_lcd 0)
pico_enable_stdio_usb(pico_lcd 1)

add_library(st7789api
    ST7789api.cpp
)
target_precompile_headers(st7789api 
    PUBLIC 
        "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/ST7789api.hpp>"
)
target_link_libraries(st7789api
    pico_stdlib
    pico_rand
    hardware_spi
)

# FetchContent_MakeAvailable(etft)

# Add the standard library to the build
target_link_libraries(pico_lcd
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(pico_lcd PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(pico_lcd 
        hardware_spi
        st7789api
        )

pico_add_extra_outputs(pico_lcd)

