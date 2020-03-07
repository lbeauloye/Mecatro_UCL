#include <cstdio>
#include <stdlib.h> 
#include <iostream>
#include <string.h>
#include <unistd.h>
#include "can.hpp"
#include "motor_card.hpp"

int main(int argc, char *argv[])
{
    printf("hello\n");
    send_message("408", "1E4040");
    motor_card *test = new motor_card("408", 1);
    test->ctrl_motor(1);
    test->set_voltage(50);
}