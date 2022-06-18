#include<Arduino.h>
#include<rtc.h>

#include "LiquidCrystal_I2C.h"

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

enum AutowateringTimeSetAction {
	NOP,
	HOUR_0,
	HOUR_1,
	MINUTE_0,
	MINUTE_1,
	ACTION_BOUNDARY
};

LiquidCrystal_I2C lcd(0x27,16,2);

void
setup()
{
	Wire.begin();
	lcd.init();
	lcd.backlight();
	/* FIXME hardcoded datetime value */
	ds1307_save(DAY,10);
	ds1307_save(MONTH,6);
	ds1307_save(YEAR,0x22);
	ds1307_save(HOUR,8);
	ds1307_save(MINUTE,0x59);
	ds1307_save(SECOND,0x50);
	ds1307_save(DATE,2);
	pinMode(PIN_HUMIDITY,INPUT);
	pinMode(PIN_VALVE,OUTPUT);
	pinMode(PIN_BUTTON_0,INPUT);
	pinMode(PIN_BUTTON_1,INPUT);
	pinMode(PIN_BUTTON_2,INPUT);
	digitalWrite(PIN_VALVE,LOW);
}

static uint8_t
bcd(uint8_t z)
{
	return ((z / 10) << 4) + (z % 10);
}

static String
bcd_to_string(uint8_t d)
{
	return String(d >> 4) + String(d & 0x0f);
}

static void
autowatering_time_set_value(enum DS1037Datetime type, uint8_t *value)
{
	uint8_t limit;

	/* TODO add more limits */
	switch (type) {
	case MINUTE:
		*limit = 60;
		break;
	case HOUR:
		*limit = 24;
		break;
	default:
		*limit = 0;
	}

	if (digitalRead(PIN_BUTTON_0) == HIGH) {
		return;
	}
	if (digitalRead(PIN_BUTTON_1) == HIGH) {
		if (*value < limit) {
			*value = *value + 1;
		}
	}
	if (digitalRead(PIN_BUTTON_2) == HIGH) {
		if (*value > 0) {
			*value = *value - 1;
		}
	}
}

static void
autowatering_time_set(void)
{
	static enum AutowateringTimeSetAction a = NOP;

	if (digitalRead(PIN_BUTTON_0) == HIGH) {
		a = (a + 1) % ACTION_BOUNDARY;
	}
	switch (a) {
	case NOP:
		break;
	case HOUR_0:
		lcd.setCursor(0,0);
		lcd.print("Gio tuoi 1: " + schedule_0_hour);
		autowatering_time_set_value(HOUR,&schedule_0_hour);
		break;
	case MINUTE_0:
		lcd.setCursor(0,0);
		lcd.print("Phut tuoi 1: " + schedule_0_minute);
		autowatering_time_set_value(MINUTE,&schedule_0_minute);
		break;
	case HOUR_1:
		lcd.setCursor(0,0);
		lcd.print("Gio tuoi 2: " + schedule_1_hour);
		autowatering_time_set_value(HOUR,&schedule_1_hour);
		break;
	case MINUTE_1:
		lcd.setCursor(0,0);
		lcd.print("Phut tuoi 2: " + schedule_1_minute);
		autowatering_time_set_value(MINUTE,&schedule_1_minute);
	}
}

void
loop()
{
	uint8_t humidity = map(analogRead(PIN_HUMIDITY),0,1023,100,0);

	bool is_critical = ds1307_load(SECOND) <= 2 && humidity < HUMIDITY_CRITICAL;
	bool on_hour_0   = ds1307_load(HOUR)   == bcd(schedule_0_hour);
	bool on_minute_0 = ds1307_load(MINUTE) == bcd(schedule_0_minute);
	bool on_hour_1   = ds1307_load(HOUR)   == bcd(schedule_0_hour);
	bool on_minute_1 = ds1307_load(MINUTE) == bcd(schedule_0_minute);

	bool on_schedule_0 = on_hour_0 && on_minute_0;
	bool on_schedule_1 = on_hour_1 && on_minute_1;

	bool on_schedule = on_schedule_0 || on_schedule_1;

	lcd.setCursor(0,0);
	lcd.print(bcd_to_string(rtc_load(HOUR))
	  + ":" + bcd_to_string(rtc_load(MINUTE))
	  + ":" + bcd_to_string(rtc_load(SECOND));
	lcd.setCursor(0,1);
	lcd.print("Do am: " + String(humidity) + "%");

	autowatering_time_set();

	/* FIXME no sleeping mode */
	if (is_critical || (on_schedule && humidity < HUMIDITY_GOOD)) {
		digitalWrite(PIN_VALVE,HIGH);
	} else {
		digitalWrite(PIN_VALVE,LOW);
	}

	delay(200);
}
