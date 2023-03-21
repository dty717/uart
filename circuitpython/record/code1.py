import digitalio
import time
import board
import busio
import logger
#     logging = False


led = digitalio.DigitalInOut(board.GP25)
led.direction = digitalio.Direction.OUTPUT
for i in range(3):
    led.value = 1
    time.sleep(0.5)
    led.value = 0
    time.sleep(0.5)

print("logging:"+str(logger.logging))

en = digitalio.DigitalInOut(board.GP2)
en.direction = digitalio.Direction.OUTPUT
en.value = 0
uart = busio.UART(board.GP0, board.GP1, baudrate=115200, stop=1, parity=None)
uart.read()

logger.log("program start\r\n")
while True:
    try:
        readBuf = uart.read()
        if readBuf:
            print(readBuf)
            logger.log(readBuf.decode())
            led.value = 1
            time.sleep(0.5)
            led.value = 0
    except Exception as e:
        print(e)
        logger.log(str(e))
    # print(time.localtime())
