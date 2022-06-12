#include <Wire.h>

class DS1307{
    private:
        uint8_t _addr;
    public:
        DS1307(uint8_t address);
        void SetTime(uint8_t date,uint8_t month ,uint8_t year,uint8_t hour ,uint8_t min,uint8_t sec,uint8_t DoW);
        uint8_t ReadSecond();
        uint8_t ReadMinute();
        uint8_t ReadHour24();
        uint8_t ReadDate();
        uint8_t ReadMonth();
        uint8_t ReadYear();
        uint8_t ReadDayOfWeek();

};