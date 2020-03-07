#include <cstdio>
#include <stdlib.h> 
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <wiringPiSPI.h>
#include "IO/COM/CAN/CAN.hh"
#include "IO/COM/SPI/Specific/SPI_CAN.hh"
#include "IO/COM/SPI/SPI.hh"
#include "IO/beacon.hh"



#define CAN_BR 125e3

int main(int argc, char *argv[])
{
	if(argc == 1){
		CAN *can;
		can = new CAN(CAN_BR);
		can->configure();

		can->ctrl_led(1);
		can->ctrl_motor(1);
		can->push_TowDC(0);
		can->push_PropDC(20,20);
		sleep(5);
		can->ctrl_motor(0);
	}
	else if(argc == 2){
		std::string type;
		type.assign(argv[1]);
		if(type == "beacon")
		{
			follow_beacon();
		}
		if(type == "stop"){
			CAN *can;
			can = new CAN(CAN_BR);
			can->configure();
			can->push_TowDC(0);
			can->push_PropDC(0,0);
			can->ctrl_motor(0);
		}
	}
	else if(argc == 3){
		std::string type, order;
		type.assign(argv[1]);
		order.assign(argv[2]);
		if(type == "beacon" && order == "drunk")
		{
			follow_beacon_drunk();
		}
	}
	else if(argc == 5){
		CAN *can;
		can = new CAN(CAN_BR);
		can->configure();

		std::string type, direction, tmp;
		type.assign(argv[1]);
		direction.assign(argv[2]);
		tmp.assign(argv[3]);
		int speedL = atoi(argv[3]);
		tmp.assign(argv[4]);
		int speedR = atoi(argv[4]);
		printf("%d %d \n\n", speedL, speedR);

		if(type == "manual"){
			printf("good\n");
			printf("%s", direction.c_str());
			if(direction == "UP"){
				printf("hello there");
				can->ctrl_motor(1);
				sleep(0.3);
				can->push_PropDC(speedL, speedR);
				
			} else if (direction == "NO"){
				can->ctrl_motor(0);
			} else {
				can->ctrl_motor(0);
			}
		} else {
			return 0;
		}
	}
	else {
		printf("no");
	}
	printf("%d",argc);
}

