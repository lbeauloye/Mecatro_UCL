import RPi.GPIO as GPIO
from time import sleep, time
import spidev
import math


def get_speed(a):
    ratio = 5.8 * 4 
    left = get_int(a[3], a[4])
    right = get_int(a[1], a[2])

    max = 2048 #512
    T = 1.0/2000
    return([2*math.pi*left/(max*T*ratio), 2*math.pi*right/(max*T*ratio)])


def get_int(a, b):
    neg = (a / 255.0) >= 0.5
    c = a * 2**8 + b
    if neg:
        c = -(65536 - c + 1)

    return c


MyARM_ResetPin = 19  # Pin 4 of connector = BCM19 = GPIO[1]

MySPI_FPGA = spidev.SpiDev()
MySPI_FPGA.open(0, 0)
MySPI_FPGA.max_speed_hz = 500000

GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
GPIO.setup(MyARM_ResetPin, GPIO.OUT)

with open('speed_robot.txt', 'w+') as f:
    ref = time()
    while True:
        ToSPI = [0x00, 0xFF, 0xFF, 0xFF, 0xFF]
        FromSPI = MySPI_FPGA.xfer2(ToSPI)
        (speed_FL, speed_RL) = get_speed(FromSPI)
        print(FromSPI)
        ToSPI = [0x01, 0xFF, 0xFF, 0xFF, 0xFF]
        FromSPI = MySPI_FPGA.xfer2(ToSPI)
        print(FromSPI)
        (speed_FR, speed_RR) = get_speed(FromSPI)
        f.seek(0)
        f.write(str(speed_FL) + ' ' + str(speed_RL) + ' ' + str(speed_FR) + ' ' + str(speed_RR))
        print(str(speed_FL) + ' ' + str(speed_RL) + ' ' + str(speed_FR) + ' ' + str(speed_RR))
        f.truncate()
        sleep(0.01)

f.close()
