import board
import busio
import digitalio
import time
import usb_cdc

# For most CircuitPython boards:
led = digitalio.DigitalInOut(board.LED)
# For QT Py M0:
# led = digitalio.DigitalInOut(board.SCK)
led.direction = digitalio.Direction.OUTPUT

buf = bytearray(8)

arr = bytes([1,3,118,0,160,0,180,3,22,0,170,0,170,0,170,0,160,0,0,0,0,0,0,0,0,0,0,0,0,0,160,0,160,0,150,0,150,0,150,0,150,0,160,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,2,0,1,0,1,0,1,0,0,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,0,0,2,0,0,0,2,0,2,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,151])

while True:
    usb_cdc.data.readinto(buf)
    usb_cdc.data.write(arr)
    print(buf)
    led.value = 1
    time.sleep(0.2)
    led.value = 0
    time.sleep(0.2)
    led.value = 1
    time.sleep(0.2)
    led.value = 0
