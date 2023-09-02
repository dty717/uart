

"""CircuitPython Essentials UART Serial example"""
import board
import busio
import digitalio
import random
import time

testRandomLen = 50
testBaudrate = 9600

# For most CircuitPython boards:
led = digitalio.DigitalInOut(board.LED)
# For QT Py M0:
led.direction = digitalio.Direction.OUTPUT

uart0 = busio.UART(board.GP0, board.GP1, baudrate=testBaudrate,stop = 1,parity = None)
en0 = digitalio.DigitalInOut(board.GP2)
en0.direction = digitalio.Direction.OUTPUT

uart1 = busio.UART(board.GP4, board.GP5, baudrate=testBaudrate,stop = 1,parity = None)
en1 = digitalio.DigitalInOut(board.GP3)
en1.direction = digitalio.Direction.OUTPUT

en0.value = 0
en1.value = 0

led1 = digitalio.DigitalInOut(board.GP11)
led1.direction = digitalio.Direction.OUTPUT
led2 = digitalio.DigitalInOut(board.GP12)
led2.direction = digitalio.Direction.OUTPUT
led3 = digitalio.DigitalInOut(board.GP13)
led3.direction = digitalio.Direction.OUTPUT
led4 = digitalio.DigitalInOut(board.GP14)
led4.direction = digitalio.Direction.OUTPUT

def wr0(buf):
    en0.value = 1
    uart0.write(buf)

def r0():
    en0.value = 0
    return uart0.read(100)

def wr1(buf):
    en1.value = 1
    uart1.write(buf)

def r1():
    en1.value = 0
    return uart1.read(100)

def randomString():
    strLen = int(testRandomLen*random.random()) + 1
    randomStr = ''
    for i in range(strLen):
        randomVal =  int(52 * random.random())
        if randomVal >= 26:
            randomVal - 26
            randomStr += chr(randomVal - 26 + 0x61)
        else:
            randomStr += chr(randomVal + 0x41)
    return randomStr

while True:
    randomStringBuf = randomString().encode()
    en1.value = 0
    wr0(randomStringBuf)
    uart1RecBuf = r1()
    print(uart1RecBuf)
    if uart1RecBuf == randomStringBuf or (uart1RecBuf and len(uart1RecBuf) > 0 and uart1RecBuf[0]==0 and uart1RecBuf[1:] == randomStringBuf):
        led1.value = 1
        led2.value = 1
    else:
        print("test uart1 receive fail.")
        print("Or test uart0 send fail.")
    randomStringBuf = randomString().encode()
    en0.value = 0
    wr1(randomStringBuf)
    uart0RecBuf = r0()
    print(uart0RecBuf)
    if uart0RecBuf == randomStringBuf or (uart0RecBuf and len(uart0RecBuf) > 0 and uart0RecBuf[0]==0 and uart0RecBuf[1:] == randomStringBuf):
        led3.value = 1
        led4.value = 1
    else:
        print("test uart0 receive fail.")
        print("Or test uart1 send fail.")
    time.sleep(1)
    led.value =  led1.value and led2.value and led3.value and led4.value 
    led1.value = 0
    led2.value = 0
    led3.value = 0
    led4.value = 0

