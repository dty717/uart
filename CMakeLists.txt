cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(uart_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()
add_executable(uart
  uart.c
  device.c
  modbus.c
  modbusRTU.c
  crc.c
  J212.c
  flash.c
  common/handler.c
)

# enable usb output, disable uart output
pico_enable_stdio_usb(uart 1)
pico_enable_stdio_uart(uart 0)

pico_add_extra_outputs(uart)
target_link_libraries(uart pico_stdlib pico_multicore hardware_adc hardware_spi hardware_flash)