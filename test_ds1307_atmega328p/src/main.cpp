#include <Arduino.h>
#include "LiquidCrystal_I2C.h"
#include "DS1307.h"

/* FIXME stub pin numbers */
#define PIN_BUTTON_0 B0
#define PIN_BUTTON_1 B1
#define PIN_BUTTON_2 B2

#define PIN_HUMIDITY A3
#define PIN_VALVE    9
#define ADDRESS_RTC  0x68

#define HUMIDITY_CRITICAL 25;
#define HUMIDITY_GOOD     70;

static uint8_t schedule_0_hour   = 9;
static uint8_t schedule_0_minute = 0;
static uint8_t schedule_1_hour   = 15;
static uint8_t schedule_1_minute = 0;

enum Datetime {
	SECOND      = 0,
	MINUTE      = 1,
	HOUR        = 2,
	DAY_OF_WEEK = 3,
	DAY         = 4,
	MONTH       = 5,
	YEAR        = 6
};

enum AutowateringTimeSetAction {
	NOP,
	HOUR_0,
	HOUR_1,
	MINUTE_0,
	MINUTE_1
};

LiquidCrystal_I2C lcd(0x27,16,2);
DS1307 rtc(0x68);

void
setup()
{
	Wire.begin();
	lcd.init();
	lcd.backlight();
	/* FIXME hardcoded datetime value */
	rtc.SetTime(10,6,22,8,59,50,2);
	pinMode(PIN_HUMIDITY,INPUT);
	pinMode(PIN_VALVE,OUTPUT);
	pinMode(PIN_BUTTON_0,INPUT);
	pinMode(PIN_BUTTON_1,INPUT);
	pinMode(PIN_BUTTON_2,INPUT);
	digitalWrite(PIN_VALVE,LOW);
}

static String
bcd_to_string(uint8_t v)
{
	return String(v & 0xf0) + String(v & 0x0f);
}

static void
autowatering_time_set_value(uint8_t *v)
{
	/* FIXME slow functions */
	lcd.clear();
	lcd.print(String(*v));

	if (digitalRead(PIN_BUTTON_0) == HIGH) {
		return;
	}
	/* FIXME no bound checking */
	if (digitalRead(PIN_BUTTON_1) == HIGH) {
		*v = *v + 1;
	}
	if (digitalRead(PIN_BUTTON_2) == HIGH) {
		*v = *v - 1;
	}
}

static void
autowatering_time_set(void)
{
	static enum AutowateringTimeSetAction a = NOP;

	if (digitalRead(PIN_BUTTON_0) == HIGH) {
		/* masochism */
		switch (a) {
		case NOP:
			a = HOUR_0;
			break;
		case HOUR_0:
			a = MINUTE_0;
			break;
		case MINUTE_0:
			a = HOUR_1;
			break;
		case HOUR_1:
			a = MINUTE_1;
			break;
		case MINUTE_1:
			a = NOP;
		}
	}
	switch (a) {
	case NOP:
		break;
	case HOUR_0:
		autowatering_time_set_value(&schedule_0_hour);
		break;
	case MINUTE_0:
		autowatering_time_set_value(&schedule_0_minute);
		break;
	case HOUR_1:
		autowatering_time_set_value(&schedule_1_hour);
		break;
	case MINUTE_1:
		autowatering_time_set_value(&schedule_1_minute);
	}
}

static uint8_t
rtc_load(enum Datetime type)
{
	Wire.beginTransmission(ADDRESS_RTC);
	Wire.write(type);
	Wire.endTransmission();
	Wire.requestFrom(ADDRESS_RTC,1);
	if (type == HOUR) {
		return Wire.read() & 0x3f;
	}
	return Wire.read();
}

void
loop()
{
	uint8_t humidity = map(analogRead(PIN_HUMIDITY),0,1023,100,0);

	lcd.setCursor(4,0);
	lcd.print(bcd_to_string(rtc.ReadHour24())
	  + ":" + bcd_to_string(rtc.ReadMinute())
	  + ":" + bcd_to_string(rtc.ReadSecond()));
	lcd.setCursor(0,1);
	lcd.print("Do am: " + String(humidity) + "%");

	/* FIXME no sleeping mode */
	if (((rtc_load(SECOND) <= 2 && humidity < HUMIDITY_CRITICAL)
	  || (rtc_load(HOUR) == SCHEDULE_0_HOUR && rtc_load(MINUTE) == SCHEDULE_0_MINUTE)
	  || (rtc_load(HOUR) == SCHEDULE_1_HOUR && rtc_load(MINUTE) == SCHEDULE_1_MINUTE))
	 && humidity <= HUMIDITY_GOOD) {
		digitalWrite(PIN_VALVE,HIGH);
	} else {
		digitalWrite(PIN_VALVE,LOW);
	}

	delay(200);
}
