
# SPDX-FileCopyrightText: 2018 Kattni Rembor for Adafruit Industries
#
# SPDX-License-Identifier: MIT

"""CircuitPython Essentials UART Serial example"""
import board
import busio
import digitalio
import time
# For most CircuitPython boards:
led = digitalio.DigitalInOut(board.LED)
led.direction = digitalio.Direction.OUTPUT
led1 = digitalio.DigitalInOut(board.GP11)
led1.direction = digitalio.Direction.OUTPUT
led2 = digitalio.DigitalInOut(board.GP12)
led2.direction = digitalio.Direction.OUTPUT
led3 = digitalio.DigitalInOut(board.GP13)
led3.direction = digitalio.Direction.OUTPUT
led4 = digitalio.DigitalInOut(board.GP14)
led4.direction = digitalio.Direction.OUTPUT

# init
led.value = 1
time.sleep(0.2)
led1.value = 1
time.sleep(0.2)
led2.value = 1
time.sleep(0.2)
led3.value = 1
time.sleep(0.2)
led4.value = 1
time.sleep(0.2)
led.value = 0
time.sleep(0.2)
led1.value = 0
time.sleep(0.2)
led2.value = 0
time.sleep(0.2)
led3.value = 0
time.sleep(0.2)
led4.value = 0
time.sleep(0.2)

# init
for i in range(20):
    print("init board:"+str(i))
    led.value = 0
    time.sleep(1)
    led.value = 1
    time.sleep(1)

uart = busio.UART(board.GP0, board.GP1, baudrate=9600,timeout = 0.5)
# print(uart.read(1000))
