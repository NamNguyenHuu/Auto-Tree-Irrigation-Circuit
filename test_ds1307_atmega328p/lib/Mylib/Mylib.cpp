#include <Mylib.h>

uint8_t ReadHumidity(uint8_t pin){
    return map(analogRead(pin),0,1023,100,0);
}

String FixValToDisplay(uint8_t val){
    if(val<10) return "0"+String(val);
    else return String(val);
}