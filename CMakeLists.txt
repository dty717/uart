# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "C:/Users/18751/Desktop/work/Electronics/rasberrayPi/pico/pico-sdk")

set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(uart C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(uart
  uart.c
  uart_suqian_receiving.c
  uart_suqian_sending.c
  uart_suzhou.c
  uart_jiangning.c
  uart_test.c
  uart_test_gprs.c
  uart_test2.c
  uart_kunshan.c
  uart_hubei.c
  uart_tibet.c
  uart_drone.c
  uart_CLO3.c
  uart_as_pio.c
  crc.c
  http.c
  beidou.c
  device.c
  modbus.c
  modbusRTU.c
  J212.c
  flash.c
  gprs.c
  gps.c
  common/handler.c
)

pico_generate_pio_header(uart ${CMAKE_CURRENT_LIST_DIR}/uart_rx.pio)

pico_set_program_name(uart "uart")
pico_set_program_version(uart "0.1")

# enable usb output, disable uart output
pico_enable_stdio_usb(uart 1)
pico_enable_stdio_uart(uart 0)

# Add the standard include files to the build
target_include_directories(uart PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

# Add any user requested libraries
target_link_libraries(uart PRIVATE pico_stdlib pico_multicore hardware_adc hardware_gpio hardware_rtc hardware_spi hardware_flash hardware_pio pico_async_context_threadsafe_background)


pico_add_extra_outputs(uart)

