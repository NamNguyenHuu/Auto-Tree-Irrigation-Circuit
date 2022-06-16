#include <Arduino.h>
#include "LiquidCrystal_I2C.h"
#include "DS1307.h"
#include "Mylib.h"

#define pin_hm A3  // pin doc du lieu tu cam bien do am dat
#define pin_valve 9 // pin kich van tuoi

/* FIXME stub pin numbers */
#define PIN_BUTTON_0 B0
#define PIN_BUTTON_1 B1
#define PIN_BUTTON_2 B2

enum AutowateringTimeSetAction {
	NOP,
	HOUR_0,
	HOUR_1,
	MINUTE_0,
	MINUTE_1
};

LiquidCrystal_I2C lcd(0x27,16,2);
DS1307 rtc(0x68);

// Dat thoi gian tuoi cay 
/* Thoi gian tuoi lan 1 */
uint8_t time1_hr = 9;
uint8_t time1_mn = 0;
/* Thoi gian tuoi lan 2 */
uint8_t time2_hr = 15;
uint8_t time2_mn = 0;

// Do am can tuoi (nguong duoi')
uint8_t hm1 = 20;

// Do am ngung tuoi (nguong tren)
uint8_t hm2 = 60;

//long long last_check;

/* on_off 
      = 0: van tat, san sang kiem tra thoi gian va do am.
      = 1: da kiem tra, xac nhan phu hop dieu kien, chuan bi tuoi.
      = 2: dang tuoi.
      */ 
uint8_t on_off = 0;

void setup() {
  Wire.begin();
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  /* Phuong thuc SetTime - Thu tu tham so: Ngay, thang, nam, gio, phut, giay, thu'*/
  rtc.SetTime(10,6,22,8,59,50,2);
  pinMode(pin_hm,INPUT);
  pinMode(pin_valve,OUTPUT);
  // chac chan rang ban dau chan kich van la LOW
  digitalWrite(pin_valve,LOW);

	pinMode(PIN_BUTTON_0,INPUT);
	pinMode(PIN_BUTTON_1,INPUT);
	pinMode(PIN_BUTTON_2,INPUT);
}

// Ham hien thi gio:phut:giay len lcd
void Display(){
  lcd.setCursor(4,0);
  lcd.print(FixValToDisplay(rtc.ReadHour24())+ ":" 
              + FixValToDisplay(rtc.ReadMinute()) + ":"
                  + FixValToDisplay(rtc.ReadSecond()));
  lcd.setCursor(0,1);
  //last_check = millis();
  
  lcd.print("DO AM: " + FixValToDisplay(ReadHumidity(pin_hm)) + "%");
}

// Ham kiem tra thoi gian va do am de kich van 
void CheckOnActive(){
  if(on_off == 0){
    // Khi kiem tra vao dau thoi gian hen gio-- va --- troi khong co mua (do am thap hon hm2)
    Serial.println(rtc.ReadSecond());
    if(((2 - rtc.ReadSecond()) >= 0) && ReadHumidity(pin_hm) < hm2){
      Serial.print("on1");
      // neu gio va phut bang hoac do am be hon hm1
      if( (rtc.ReadHour24() == time1_hr && rtc.ReadMinute() == time1_mn) || 
      (rtc.ReadHour24() == time2_hr && rtc.ReadMinute() == time2_mn) || ReadHumidity(pin_hm) < hm1) 
        // san sang kich van 
        on_off = 1;
        Serial.println(on_off);
    } 
  }
  else if (on_off == 1) {
    // bat dau kich van
    digitalWrite(pin_valve,HIGH);
    // co` thong bao van dang hoat dong
    on_off = 2;
  }
  else if (on_off == 2) {
    // khi van dang hoat dong , neu do am > hm2
    if(ReadHumidity(pin_hm) > hm2) {
      // tat van
      digitalWrite(pin_valve,LOW);
      // thong bao van dang tat, san sang kiem tra cho lan tiep theo
      on_off = 0;
    }
  }
}

void
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

void
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
		autowatering_time_set_value(&time1_hr);
		break;
	case MINUTE_0:
		autowatering_time_set_value(&time1_mn);
		break;
	case HOUR_1:
		autowatering_time_set_value(&time2_hr);
		break;
	case MINUTE_1:
		autowatering_time_set_value(&time2_mn);
	}
}

void loop() {
  Display();
  CheckOnActive();
  delay(200);
  // put your main code here, to run repeatedly:
}

