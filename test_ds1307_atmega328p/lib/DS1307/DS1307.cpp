#include "DS1307.h"

/* Chuyển từ format BCD (Binary-Coded Decimal) sang Decimal */
int bcd2dec(uint8_t num){
        return ((num/16 * 10) + (num % 16));
}

/* Chuyển từ Decimal sang BCD */
int dec2bcd(uint8_t num){
        return ((num/10 * 16) + (num % 10));
}

DS1307::DS1307(uint8_t add){
    this->_addr = add;
}

void DS1307::SetTime(uint8_t date,uint8_t month ,uint8_t year,uint8_t hour ,uint8_t min,uint8_t sec,uint8_t DoW){
    Wire.beginTransmission(_addr);
    Wire.write(0x00); // đặt lại pointer
    Wire.write(dec2bcd(sec));
    Wire.write(dec2bcd(min));
    Wire.write(dec2bcd(hour));
    Wire.write(dec2bcd(DoW)); // day of week: Sunday = 1, Saturday = 7
    Wire.write(dec2bcd(date)); 
    Wire.write(dec2bcd(month));
    Wire.write(dec2bcd(year));
    Wire.endTransmission();
}

uint8_t DS1307::ReadSecond(){
    Wire.beginTransmission(_addr);
    Wire.write(0x00); // đặt lại pointer
    Wire.endTransmission();
    Wire.requestFrom(_addr,1);
    return bcd2dec(Wire.read());
}

uint8_t DS1307::ReadMinute(){
    Wire.beginTransmission(_addr);
    Wire.write(0x01); // đặt lại pointer
    Wire.endTransmission();
    Wire.requestFrom(_addr,1);
    return bcd2dec(Wire.read());
}

uint8_t DS1307::ReadHour24(){
    Wire.beginTransmission(_addr);
    Wire.write(0x02); // đặt lại pointer
    Wire.endTransmission();
    Wire.requestFrom(_addr,1);
    return bcd2dec(Wire.read() & 0x3f);
}

uint8_t DS1307::ReadDayOfWeek(){
    Wire.beginTransmission(_addr);
    Wire.write(0x03); // đặt lại pointer
    Wire.endTransmission();
    Wire.requestFrom(_addr,1);
    return bcd2dec(Wire.read());
}

uint8_t DS1307::ReadDate(){
    Wire.beginTransmission(_addr);
    Wire.write(0x04); // đặt lại pointer
    Wire.endTransmission();
    Wire.requestFrom(_addr,1);
    return bcd2dec(Wire.read());
}

uint8_t DS1307::ReadMonth(){
    Wire.beginTransmission(_addr);
    Wire.write(0x05); // đặt lại pointer
    Wire.endTransmission();
    Wire.requestFrom(_addr,1);
    return bcd2dec(Wire.read());
}

uint8_t DS1307::ReadYear(){
    Wire.beginTransmission(_addr);
    Wire.write(0x06); // đặt lại pointer
    Wire.endTransmission();
    Wire.requestFrom(_addr,1);
    return bcd2dec(Wire.read());
}