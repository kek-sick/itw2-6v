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
#define WF_SECOND_TAP 600   //время окончания ожидания второго тапа
#define WF_DEBOUNCE 300		//время окончания ожидания дребезга переключателя

iarduino_RTC clock(RTC_DS1302, 10, 12, 11);

void lamp1();
void lamp2();
void show_s(byte, byte);
//void show_r_strg(byte*);

unsigned long time = 0, fst_tap_time = 0, GetTemp_time = 0, mills_ftt = 0;
byte a = 0,b = 0;
bool first_tap_btn_flag = false,	//if one tap happened
	second_tap_btn_flag = false,	//if two taps hepptned
	allow_secon_tap = false,		//if TRUE then second tap can be detected
	ignition = false,				//TRUE when power lines are active
	has_shown = false;				//TRUE when showing func has done
	
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
	{1,0,0,0,0,0,0,0}, // dots 18
	{0,0,1,1,1,1,0,0}, // o (upper) 19
};

byte animation[2][11] = {
	{0,0,3,5,4,2,6,8,7,1,0},
	{0,0,7,5,8,6,2,4,3,1,0}
};
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
	//show_s(16, 16);    //off all segments
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
	/*Serial.print(first_tap_btn_flag);
	Serial.print(allow_secon_tap);
	Serial.println(second_tap_btn_flag);*/
	if (mills_ftt < WF_SECOND_TAP)  //показываем анимацию
	{
		if (mills_ftt % 10 < 5)
		{
			show_anim(mills_ftt / 60, 1);
		}
		else 
		{
			show_anim(mills_ftt / 60, 2);
		}
	}
	//first tap detection
	if (!digitalRead(13) && !first_tap_btn_flag)
	{
		first_tap_btn_flag = true;
		fst_tap_time = millis();
		//show_time = fst_tap_time;
		b = GetTemp();
	}
	mills_ftt = millis() - fst_tap_time;

	//allowing to detect secont tap
	if (digitalRead(13) && !allow_secon_tap && first_tap_btn_flag && mills_ftt > WF_DEBOUNCE && mills_ftt < WF_SECOND_TAP)
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
	//showing time
	if (first_tap_btn_flag && !second_tap_btn_flag  && !has_shown && mills_ftt > WF_SECOND_TAP)
	{
		a = clock.Hours;
		while (millis() - fst_tap_time < 1500)
		{
			show_s(byte(a/10),byte(a%10));
		}
		show_s(16, 16);
		digitalWrite(2, LOW);
		digitalWrite(0, LOW);
		digitalWrite(1, LOW);
		a = clock.minutes;
		delay(300);
		while (millis() - fst_tap_time < 2800)
		{
			show_s(byte(a / 10), byte(a % 10));
		}
		show_s(16, 16);
		first_tap_btn_flag = false;
		has_shown = true;
		allow_secon_tap = false;
	}
	//showing data
	if (first_tap_btn_flag && second_tap_btn_flag && !has_shown && mills_ftt > WF_SECOND_TAP)
	{
		while (millis() - fst_tap_time < 1500)
		{
			show_s(10, 19);
		}
		while (millis() - fst_tap_time < 2800)
		{
			show_s(byte(b / 10), byte(b % 10));
		}
		show_s(16, 16);
		first_tap_btn_flag = false;
		has_shown = true;
		allow_secon_tap = false;
		second_tap_btn_flag = false;
	}

	if (has_shown && ignition)
	{
		has_shown = false;
		ignition = false;
		digitalWrite(HIGH_VOLTAGE, LOW);
		digitalWrite(FILAMENT, LOW);
		for (byte i = 0; i < 10; i++) //вырубаем все выводы, чтобы не тратить энергию
		{
			digitalWrite(i, LOW);
		}
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

void show_anim(byte frame, byte lamp) {
	byte seg[3];
	if (lamp == 1) {
		lamp1();
	}
	else
	{
		lamp2();
	}
	for (size_t i = 0; i < 3; i++)seg[i] = animation[lamp - 1][frame + i];  //обозначаем нужные сегменты в этом кадре
	for (byte i = 2; i < 10; i++)		// вырубаем все сегменты
	{
		digitalWrite(i,HIGH);
	}
	if(seg[1]!=0){digitalWrite(seg[1]+1,LOW);}  //включаем нужные 3
	if(seg[2]!=0){digitalWrite(seg[2]+1,LOW);}
	if(seg[0]!=0){digitalWrite(seg[0]+1,LOW);}
}

void show_s(byte symbol_write_1, byte symbol_write_2) {
	lamp1();
	for (short i = 2; i < 10; i++){
		digitalWrite(i, !symb[symbol_write_1][i - 2]);
	}
	delay(5);
	lamp2();
	for (short j = 2; j < 10; j++) {
		digitalWrite(j, !symb[symbol_write_2][j - 2]);
	}
	delay(5);
}

/*void show_r_strg(byte* strg) {
	for (byte i = 0; i <=sizeof(strg); i++){
		time = millis();
		while (millis()-time < 750){
			show_s(strg[i], strg[i + 1]);
		}
	}
}*/

byte GetTemp(void)
{
		unsigned int wADC;
		double t;
		ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
		ADCSRA |= _BV(ADEN);  // enable the ADC
		delay(20);            // wait for voltages to become stable.
		ADCSRA |= _BV(ADSC);  // Start the ADC
		// Detect end-of-conversion
		while (bit_is_set(ADCSRA, ADSC));
		// Reading register "ADCW" takes care of how to read ADCL and ADCH.
		wADC = ADCW;
		// The offset of 324.31 could be wrong. It is just an indication.
		t = (wADC - 324.31) / 1.22;
		// The returned temperature is in degrees Celsius.
		return (byte(t));
}
