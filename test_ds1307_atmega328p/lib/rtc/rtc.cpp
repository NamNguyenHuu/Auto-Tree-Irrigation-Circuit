#include"rtc.h"

static const uint8_t ADDRESS = 0x68;

uint8_t
ds1307_load(enum DS1307Datetime type)
{
	Wire.beginTransmission(ADDRESS);
	Wire.write(type);
	Wire.endTransmission();
	Wire.requestFrom(ADDRESS,1);
	if (type == HOUR) {
		return Wire.read() & 0x3f;
	}
	return Wire.read();
}

void
ds1307_save(enum DS1307Datetime type,uint8_t * value)
{
	Wire.beginTransmission(ADDRESS);
	Wire.write(type);
	Wire.write(*value);
	Wire.endTransmission();
}
