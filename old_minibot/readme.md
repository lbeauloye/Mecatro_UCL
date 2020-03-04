# Compile
```sh
g++ -o main IO/COM/CAN/CAN.cc IO/COM/SPI/Specific/SPI_DE0.cc IO/COM/SPI/Specific/SPI_CAN.cc IO/COM/SPI/SPI.cc IO/beacon.cc IO/speed_controller.cc main.cc -lwiringPi
```