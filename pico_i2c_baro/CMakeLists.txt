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
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico_i2c_baro C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(pico_i2c_baro pico_i2c_baro.c )

pico_set_program_name(pico_i2c_baro "pico_i2c_baro")
pico_set_program_version(pico_i2c_baro "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(pico_i2c_baro 0)
pico_enable_stdio_usb(pico_i2c_baro 1)

add_library(bmp280 bmp280.c)
target_precompile_headers(bmp280 PUBLIC bmp280.h)
target_link_libraries(bmp280
    pico_stdlib
    hardware_i2c
)

add_library(ssd1306 ssd1306.c)
target_precompile_headers(ssd1306 PUBLIC ssd1306.h)
target_link_libraries(ssd1306
    pico_stdlib
    hardware_i2c
)

# Add the standard library to the build
target_link_libraries(pico_i2c_baro
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(pico_i2c_baro PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(pico_i2c_baro 
        hardware_i2c
        bmp280
        ssd1306
        )

pico_add_extra_outputs(pico_i2c_baro)

