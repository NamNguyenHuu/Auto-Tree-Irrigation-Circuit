#include<Arduino.h>
#include<Eventually.h>

#include"LiquidCrystal_I2C.h"

#define PIN_BUTTON_0 0
#define PIN_BUTTON_1 1
#define PIN_BUTTON_2 2

#define PIN_HUMIDITY A3
#define PIN_VALVE     9

static const int HUMIDITY_CRITICAL = 25;
static const int HUMIDITY_GOOD     = 70;

static int humidity;

static int mode              = 0;
static int schedule_0_hour   = 9;
static int schedule_0_minute = 0;
static int schedule_1_hour   = 15;
static int schedule_1_minute = 0;

static const int SECOND = 0;
static const int MINUTE = 1;
static const int HOUR   = 2;
static const int DATE   = 3;
static const int DAY    = 4;
static const int MONTH  = 5;
static const int YEAR   = 6;

static const int NOP             = 0;
static const int HOUR_0          = 1;
static const int HOUR_1          = 2;
static const int MINUTE_0        = 3;
static const int MINUTE_1        = 4;
static const int ACTION_BOUNDARY = 5;

static LiquidCrystal_I2C lcd(0x27,16,2);

static EvtManager mgr;

static EvtListener * display;
static EvtListener * valve;
static EvtListener * button[3];

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

	display   = new EvtTimeListener(200,true,(EvtAction)lcd_output);
	valve     = new EvtTimeListener(1000,true,(EvtAction)valve_control);
	button[0] = new EvtPinListener(PIN_BUTTON_0,(EvtAction)mode_change);
	button[1] = new EvtPinListener(PIN_BUTTON_1,(EvtAction)time_increase);
	button[2] = new EvtPinListener(PIN_BUTTON_2,(EvtAction)time_decrease);

	mgr.addListener(display);
	mgr.addListener(valve);
	mgr.addListener(button[0]);
	mgr.addListener(button[1]);
	mgr.addListener(button[2]);
}

int
bcd(int z)
{
	return ((z / 10) << 4) + (z % 10);
}

String
bcd_to_string(int d)
{
	return String(d >> 4) + String(d & 0x0f);
}

int
ds1307_load(int type)
{
	Wire.beginTransmission(0x68);
	Wire.write(type);
	Wire.endTransmission();
	Wire.requestFrom(0x68,1);
	if (type == HOUR) {
		return Wire.read() & 0x3f;
	}
	return Wire.read();
}

void
ds1307_save(int type, int value)
{
	Wire.beginTransmission(0x68);
	Wire.write(type);
	if (type == HOUR) {
		Wire.write(value | 0x40);
	} else {
		Wire.write(value);
	}
	Wire.endTransmission();
}

bool
valve_control(void)
{
	bool on_hour_0   = ds1307_load(HOUR)   == bcd(schedule_0_hour);
	bool on_minute_0 = ds1307_load(MINUTE) == bcd(schedule_0_minute);
	bool on_hour_1   = ds1307_load(HOUR)   == bcd(schedule_0_hour);
	bool on_minute_1 = ds1307_load(MINUTE) == bcd(schedule_0_minute);

	bool on_schedule_0 = on_hour_0 && on_minute_0;
	bool on_schedule_1 = on_hour_1 && on_minute_1;

	bool on_schedule = on_schedule_0 || on_schedule_1;

	humidity = map(analogRead(PIN_HUMIDITY),0,1023,100,0);

	if (humidity < HUMIDITY_CRITICAL || (on_schedule && humidity < HUMIDITY_GOOD)) {
		digitalWrite(PIN_VALVE,HIGH);
	} else {
		digitalWrite(PIN_VALVE,LOW);
	}

	return false;
}

bool
lcd_output(void)
{
	switch (mode) {
	case NOP:
		lcd.setCursor(0,0);
		lcd.print(bcd_to_string(ds1307_load(HOUR))
		  + ":" + bcd_to_string(ds1307_load(MINUTE))
		  + ":" + bcd_to_string(ds1307_load(SECOND)));
		lcd.setCursor(0,1);
		lcd.print("Do am: " + String(humidity) + "%");
		break;
	case HOUR_0:
		lcd.setCursor(0,0);
		lcd.print("Gio tuoi 1: " + schedule_0_hour);
		break;
	case MINUTE_0:
		lcd.setCursor(0,0);
		lcd.print("Phut tuoi 1: " + schedule_0_minute);
		break;
	case HOUR_1:
		lcd.setCursor(0,0);
		lcd.print("Gio tuoi 2: " + schedule_1_hour);
		break;
	case MINUTE_1:
		lcd.setCursor(0,0);
		lcd.print("Phut tuoi 2: " + schedule_1_minute);
	}
	return true;
}

bool
mode_change(void)
{
	mode = (mode + 1) % ACTION_BOUNDARY;
	return true;
}

bool
time_increase(void)
{
	switch (mode) {
	case NOP:
		break;
	case HOUR_0:
		schedule_0_hour = (schedule_0_hour + 1) % 24;
		break;
	case MINUTE_0:
		schedule_0_minute = (schedule_0_minute + 1) % 60;
		break;
	case HOUR_1:
		schedule_1_hour = (schedule_1_hour + 1) % 24;
		break;
	case MINUTE_1:
		schedule_1_minute = (schedule_1_minute + 1) % 60;
	}
	return true;
}

bool
time_decrease(void)
{
	switch (mode) {
	case NOP:
		break;
	case HOUR_0:
		schedule_0_hour = (schedule_0_hour - 1) % 24;
		break;
	case MINUTE_0:
		schedule_0_minute = (schedule_0_minute - 1) % 60;
		break;
	case HOUR_1:
		schedule_1_hour = (schedule_1_hour - 1) % 24;
		break;
	case MINUTE_1:
		schedule_1_minute = (schedule_1_minute - 1) % 60;
	}
	return true;
}

USE_EVENTUALLY_LOOP(mgr)
