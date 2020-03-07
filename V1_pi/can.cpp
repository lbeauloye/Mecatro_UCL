#include "can.hpp"

using namespace std;

void send_message(string id, string msg){
    system(("cansend can0 " + id +"#" + msg).c_str());
}