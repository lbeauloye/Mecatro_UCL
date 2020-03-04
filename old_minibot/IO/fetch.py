import RPi.GPIO as GPIO
from time import sleep, time
import spidev
import math


def get_speed(a):
    ratio = 14 * 4 
    left = get_int(a[3], a[4])
    right = get_int(a[1], a[2])

    max = 512
    T = 1.0/2000
    return([2*math.pi*left/(max*T*ratio), 2*math.pi*right/(max*T*ratio)])


def get_int(a, b):
    neg = (a / 128) == 1
    c = a * 2**8 + b
    if neg:
        c = -(65536 - c + 1)

    return c


def get_las(a):
    End = get_int(a[3], a[4])
    Start = get_int(a[1], a[2])
    max = 7200

    min_angle = (45 + Start * 360 / max) % 360
    max_angle = (45 + End * 360 / max) % 360
    return(str(min_angle) + ' ' + str(max_angle) + '\n')


MyARM_ResetPin = 19  # Pin 4 of connector = BCM19 = GPIO[1]

MySPI_FPGA = spidev.SpiDev()
MySPI_FPGA.open(0, 0)
MySPI_FPGA.max_speed_hz = 500000

GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
GPIO.setup(MyARM_ResetPin, GPIO.OUT)

with open('data_robot.txt', 'w+') as f:
    ref = time()
    while True:
       
        ToSPI = [0x00, 0xFF, 0xFF, 0xFF, 0xFF]
        FromSPI = MySPI_FPGA.xfer2(ToSPI)
        speed = get_speed(FromSPI)

        ToSPI = [0x01, 0xFF, 0xFF, 0xFF, 0xFF]
        FromSPI = MySPI_FPGA.xfer2(ToSPI)
        las = get_las(FromSPI)
        f.seek(0)
        f.write(str(speed[0]) + ' ' + str(speed[1]) + ' ' + las)
        f.truncate()
        print(speed)
        print(las)
        print(str(speed[0]) + ' ' + str(speed[1]) + ' ' + las)
        sleep(0.01)

f.close()