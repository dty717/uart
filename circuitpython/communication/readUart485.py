"""CircuitPython Essentials UART Serial example"""
import board
import busio
import digitalio

# For most CircuitPython boards:
led = digitalio.DigitalInOut(board.LED)
# For QT Py M0:
# led = digitalio.DigitalInOut(board.SCK)
led.direction = digitalio.Direction.OUTPUT

en = digitalio.DigitalInOut(board.GP2)
en.direction = digitalio.Direction.OUTPUT
en.value = 0

uart = busio.UART(board.GP0, board.GP1, baudrate=9600,stop = 1,parity = None)
bufQuery = [0x01, 0x03, 0x00, 0x57, 0x00, 0x3B, 0xB5, 0xC9]

def wr(buf):
    en.value = 1
    uart.write(buf)
    en.value = 0
    print(uart.read(100))

while True:
    strLine = uart.readline()
    if strLine != "" and strLine != None:
       print(strLine)

