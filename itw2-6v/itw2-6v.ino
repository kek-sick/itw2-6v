/*
 Name:		itw2_6v.ino
 Created:	24.01.2019 20:47:13
 Author:	Ilya
*/
#include <iarduino_RTC.h>
#define MESH1 0
#define MESH2 1
#define HIGH_VOLTAGE A1
#define FILAMENT A2
#define LIGHT_SENSOR A3

iarduino_RTC clock(RTC_DS1302, 10, 12, 11);

void lamp1();
void lamp2();
void show_s(byte, byte);
//void show_r_strg(byte*);

unsigned long time = 0, fst_tap_time = 0, show_time = 0, mills_ftt = 0;
byte a = 0,b = 0;
bool first_tap_btn_flag = false,	//if one tap happened
	second_tap_btn_flag = false,	//if two taps hepptned
	allow_secon_tap = false,		//if TRUE then second tap can be detected
	ignition = false,				//TRUE when power lines are active
	time_showing = false,			//TRUE when time showing func is operating
	data_showing = false;			//TRUE when data showing func is operating
//byte ty_pidor[] = {10,13,16,11,1,12,0,15};
//byte hyi[] = {17,13,1,16};

bool symb[][8] = {
	{0,1,1,1,0,1,1,1}, // 0
	{0,1,1,0,0,0,0,0}, // 1
	{0,0,1,1,1,0,1,1}, // 2
	{0,1,1,1,1,0,0,1}, // 3
	{0,1,1,0,1,1,0,0}, // 4
	{0,1,0,1,1,1,0,1}, // 5
	{0,1,0,1,1,1,1,1}, // 6
	{0,1,1,1,0,0,0,0}, // 7
	{0,1,1,1,1,1,1,1}, // 8
	{0,1,1,1,1,1,0,1}, // 9
	{0,0,0,0,1,1,1,1}, // t  10
	{0,0,1,1,1,1,1,0}, // p  11
	{0,1,1,0,1,0,1,1}, // d  12
	{0,1,1,0,1,1,0,1}, // y  13
	{0,1,0,0,1,0,1,1}, // o  14
	{0,0,0,0,1,0,1,0}, // r  15
	{0,0,0,0,0,0,0,0}, // space  16
	{0,1,1,0,1,1,1,0}, // H  17
	{1,0,0,0,0,0,0,0}};// dots 18

// the setup function runs once when you press reset or power the board
void setup() {
	pinMode(MESH1, OUTPUT);
	pinMode(MESH2, OUTPUT);
	pinMode(2, OUTPUT);
	pinMode(3, OUTPUT);
	pinMode(4, OUTPUT);
	pinMode(5, OUTPUT);
	pinMode(6, OUTPUT);
	pinMode(7, OUTPUT);
	pinMode(8, OUTPUT);
	pinMode(9, OUTPUT);
	pinMode(13, INPUT_PULLUP);
	pinMode(HIGH_VOLTAGE, OUTPUT);
	pinMode(FILAMENT, OUTPUT);
	pinMode(LIGHT_SENSOR, INPUT);
	
	clock.begin();
	//clock.settime(0,5, 2, 1, 2, 19, 5);
	show_s(16, 16);    //say HI
	//time = millis();
	//Serial.begin(9600);
	//Serial.println(clock.minutes);
}

// the loop function runs over and over again until power down or reset
void loop() {
	/*if (millis() - time >40)
	{
		time = millis();
		a++;
		if (a > 9) { a = 0; b++; }
		if (b > 9)b = 0;	
	}
	show_s(b, a);*/

	//first tap detection
	if (!digitalRead(13) && !first_tap_btn_flag)
	{
		first_tap_btn_flag = true;
		fst_tap_time = millis();
		show_time = fst_tap_time;
	}
	mills_ftt = millis() - fst_tap_time;

	//костыль вырубающий зажиганние, пока нет функции отображения
	//if (digitalRead(13) && millis() - show_time > 1600)
	//{
	//	//digitalWrite(HIGH_VOLTAGE, LOW);
	//	//digitalWrite(FILAMENT, LOW);
	//}

	//обнулятор тапов если второго не случилось (будет убран после нормальных функций отображения)
	//if (first_tap_btn_flag && digitalRead(13) && mills_ftt > 600)  //todo убрать digitalRead чтобы.. ноо это не точно
	//{	
	//	first_tap_btn_flag = false;
	//	second_tap_btn_flag = false;
	//}

	//allowing to detect secont tap
	if (digitalRead(13) && !allow_secon_tap && first_tap_btn_flag && mills_ftt >350 && mills_ftt < 600)
	{
		allow_secon_tap = true;
	}

	//second tap detection
	if (!second_tap_btn_flag && !digitalRead(13) && allow_secon_tap/* && mills_ftt > 350*/)
	{
		second_tap_btn_flag = true;
		allow_secon_tap = false;
	}

	//включение силовых линий
	if (first_tap_btn_flag && !ignition)
	{
		ignition = true;
		digitalWrite(HIGH_VOLTAGE, HIGH);
		digitalWrite(FILAMENT, HIGH);
	}

	if (first_tap_btn_flag && !second_tap_btn_flag  && !time_showing)
	{
		time_showing = true;
	}

	if (time_showing && mills_ftt > 600)
	{
		a = clock.Hours;
		while (millis() - show_time < 1500)
		{
			show_s(byte(a/10),byte(a%10));
		}
		show_s(16, 16);
		digitalWrite(2, LOW);
		digitalWrite(0, LOW);
		digitalWrite(1, LOW);
		a = clock.minutes;
		delay(300);
		while (millis() - show_time < 2800)
		{
			show_s(byte(a / 10), byte(a % 10));
		}
		show_s(16, 16);
		first_tap_btn_flag = false;
		//second_tap_btn_flag = false;
		time_showing = false;
		allow_secon_tap = false;
	}

	if (!data_showing && !time_showing && ignition)
	{
		ignition = false;
		digitalWrite(HIGH_VOLTAGE, LOW);
		digitalWrite(FILAMENT, LOW);
	}
	//show_r_strg(hyi);
}

void lamp1() {
	digitalWrite(MESH2, HIGH);
	digitalWrite(MESH1, LOW);
}

void lamp2() {
	digitalWrite(MESH1, HIGH);
	digitalWrite(MESH2, LOW);
}

void show_s(byte symbol_write_1, byte symbol_write_2) {
	lamp1();
	for (short i = 2; i < 10; i++){
		digitalWrite(i, !symb[symbol_write_1][i - 2]);
	}
	delay(4);
	lamp2();
	for (short j = 2; j < 10; j++) {
		digitalWrite(j, !symb[symbol_write_2][j - 2]);
	}
	delay(4);
}

/*void show_r_strg(byte* strg) {
	for (byte i = 0; i <=sizeof(strg); i++){
		time = millis();
		while (millis()-time < 750){
			show_s(strg[i], strg[i + 1]);
		}
	}
}*/
