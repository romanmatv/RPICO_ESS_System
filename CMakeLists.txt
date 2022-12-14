cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(RPICO_ESS_System C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()


add_executable(RPICO_ESS_System
        main.cpp
        )

add_library(pico_ADXL345 ADXL345.h ADXL345.cpp)
target_link_libraries(pico_ADXL345 pico_stdlib hardware_i2c)

target_link_libraries(RPICO_ESS_System pico_ADXL345)

# enable usb output, disable uart output
pico_enable_stdio_usb(RPICO_ESS_System 1)
pico_enable_stdio_uart(RPICO_ESS_System 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(RPICO_ESS_System)