/*
 Name:		itw2_6v.ino
 Created:	24.01.2019 20:47:13
 Author:	Ilya
*/
#include <iarduino_RTC.h>
#define MESH1 0
#define MESH2 1
#define HIGH_VOLTAGE A3
#define FILAMENT A2
#define BTN_DETECT A1
#define LIGHT_SENSOR A7
#define MERCURY 2			
#define WF_SECOND_TAP 600   //время окончания ожидания второго тапа
#define WF_DEBOUNCE 300		//время окончания ожидания дребезга переключателя
#define LS_ENABLE 1			//Light Sensor Enable
#define ANIMATION 2

iarduino_RTC clock(RTC_DS1307);

void Lamp(byte);
void Show_symb(byte, byte);
//void show_r_strg(byte*);

unsigned long time = 0, fst_tap_time = 0, GetTemp_time = 0, mills_ftt = 0;
byte a = 0,temperature = 0;
bool first_tap_btn_flag = false,	//if one tap happened
	second_tap_btn_flag = false,	//if two taps hepptned
	allow_secon_tap = false,		//if TRUE then second tap can be detected
	mercury_btn_flag = false,		//TRUE if btn parcer detected high value
	ignition = false,				//TRUE when power lines are active
	has_shown = false;				//TRUE when showing func has done
volatile short btn_type = 0;
volatile bool btn_flag = false;
	
//byte ty_pidor[] = {10,13,16,11,1,12,0,15};
//byte hyi[] = {17,13,1,16};

byte symb[] = {
	B01110111, // 0
	B01100000, // 1
	B00111011, // 2
	B01111001, // 3
	B01101100, // 4
	B01011101, // 5
	B01011111, // 6
	B01110000, // 7
	B01111111, // 8
	B01111101, // 9
	B00001111, // t  10
	B00111110, // p  11
	B01101011, // d  12
	B01101101, // y  13
	B01001011, // o  14
	B00001010, // r  15
	B00000000, // space  16
	B01101110, // H  17
	B10000000, // dots 18
	B00111100 // o (upper) 19
};
//anim 1
byte animation1[2][11] = {
	{0,0,3,5,4,2,6,8,7,1,0},
	{0,0,7,5,8,6,2,4,3,1,0}
};

//anim 2
byte animation2[2][11] = {
	{0,0,3,4,6,5,2,8,7,0,0},
	{0,0,7,8,2,5,6,4,3,0,0}
};

byte animation3[2][11] = {
	{0,0,3,2,4,5,8,6,7,0,0},
	{0,0,7,6,8,5,4,2,3,0,0}
};

void Btn_detect() {
	btn_type = analogRead(BTN_DETECT);
	if (btn_type > 600)
	{
		btn_flag = true;
		//Serial.println(analogRead(BTN_DETECT));
	}
	else
	{
		btn_flag = false;
	}
}

//void Btn_release() {
//	if (btn_flag) 
//	{
//		btn_type = analogRead(BTN_DETECT);
//		btn_flag = false;
//	}
//}

// the setup function runs once when you press reset or power the board
void setup() {
	pinMode(MESH1, OUTPUT);
	pinMode(MESH2, OUTPUT);
	pinMode(MERCURY, INPUT);
	pinMode(BTN_DETECT, INPUT);
	pinMode(3, OUTPUT);
	pinMode(4, OUTPUT);
	pinMode(5, OUTPUT);
	pinMode(6, OUTPUT);
	pinMode(7, OUTPUT);
	pinMode(8, OUTPUT);
	pinMode(9, OUTPUT);
	pinMode(13, OUTPUT);			 //dots
	pinMode(HIGH_VOLTAGE, OUTPUT);
	pinMode(FILAMENT, OUTPUT);
	pinMode(LIGHT_SENSOR, INPUT);

	attachInterrupt(0, Btn_detect, CHANGE);
	//attachInterrupt(0, Btn_release, FALLING);

	for (byte i = 0; i < 10; i++) //вырубаем все выводы, чтобы не тратить энергию
	{
		if (i != 2)digitalWrite(i, LOW);
	}
	digitalWrite(13, LOW);
	//clock.begin();
	//clock.settime(0, 57, 23, 1, 7, 19, 1);
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
	
	if (btn_flag)
	{
		//Serial.println(analogRead(BTN_DETECT));
		if (btn_type > 630 && btn_type < 640)mercury_btn_flag = true;
	}
	else
	{
		mercury_btn_flag = false;
	}

	if (mills_ftt < WF_SECOND_TAP)  //показываем анимацию
	{
		if (mills_ftt % 10 < 5)
		{
			Show_anim(mills_ftt / 60, 1, ANIMATION);
		}
		else 
		{
			Show_anim(mills_ftt / 60, 2, ANIMATION);
		}
	}

	//first tap detection
	if (mercury_btn_flag && !first_tap_btn_flag)
	{
		first_tap_btn_flag = true;
		fst_tap_time = millis();
		//show_time = fst_tap_time;
		temperature = GetTemp();
		//interrupts();
		//attachInterrupt(0, Btn_release, FALLING);
	}
	mills_ftt = millis() - fst_tap_time;

	//allowing to detect secont tap
	if (!mercury_btn_flag && !allow_secon_tap && first_tap_btn_flag && mills_ftt > WF_DEBOUNCE && mills_ftt < WF_SECOND_TAP)
	{
		allow_secon_tap = true;
	}

	//second tap detection
	if (!second_tap_btn_flag && mercury_btn_flag && allow_secon_tap/* && mills_ftt > 350*/)
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
		Show_time();
		first_tap_btn_flag = false;
		has_shown = true;
		allow_secon_tap = false;
	}
	//showing data
	if (first_tap_btn_flag && second_tap_btn_flag && !has_shown && mills_ftt > WF_SECOND_TAP)
	{
		Show_temperature();
		first_tap_btn_flag = false;
		has_shown = true;
		allow_secon_tap = false;
		second_tap_btn_flag = false;
	}
	//turning off power
	if (has_shown && ignition)
	{
		has_shown = false;
		ignition = false;
		digitalWrite(HIGH_VOLTAGE, LOW);
		digitalWrite(FILAMENT, LOW);
		for (byte i = 0; i < 10; i++) //вырубаем все выводы, чтобы не тратить энергию
		{
			if(i != 2)digitalWrite(i, LOW);
		}
		digitalWrite(13, LOW);
	}
}

void Lamp(byte lamp) {
	if (lamp == 1)
	{
		digitalWrite(MESH2, HIGH);
		digitalWrite(MESH1, LOW);
	}
	if (lamp == 2)
	{
		digitalWrite(MESH1, HIGH);
		digitalWrite(MESH2, LOW);
	}
}

void Show_temperature() {
	byte tt1, tt2;
	tt1 = temperature / 10;
	tt2 = temperature % 10;
	while (millis() - fst_tap_time < 1500)
	{
		Show_symb(10, 19, 0);
	}
	while (millis() - fst_tap_time < 2800)
	{
		Show_symb(tt1, tt2, 0);
	}
	Show_symb(16, 16 , 0);
}

void Show_time() {
	clock.begin();
	byte  tt1,tt2;
	tt1 = clock.Hours / 10;
	tt2 = clock.Hours % 10;
	while (millis() - fst_tap_time < 1500)
	{
		Show_symb(tt1, tt2, 2);
	}
	Show_symb(16, 16 , 0);
	tt1 = clock.minutes / 10;
	tt2 = clock.minutes % 10;
	//delay(300);
	while (millis() - fst_tap_time < 2800)
	{
		Show_symb(tt1, tt2, 1);
	}
	Show_symb(16, 16, 0);
}

void Show_anim(byte frame, byte lamp, byte anim) {
	byte seg[3];
	Lamp(lamp);
	switch (anim) //TODO other animations
	{
	case 2:
		for (size_t i = 0; i < 3; i++)seg[i] = animation2[lamp - 1][frame + i];
		break;
	case 3:
		for (size_t i = 0; i < 3; i++)seg[i] = animation3[lamp - 1][2*frame + i];
		break;
	default:
		for (size_t i = 0; i < 3; i++)seg[i] = animation1[lamp - 1][frame + i];  //обозначаем нужные сегменты в этом кадре
		break;
	}
	for (byte i = 3; i < 10; i++)		// вырубаем все сегменты
	{
		digitalWrite(i,HIGH);
	}
	if(seg[1]!=0){digitalWrite(seg[1]+1,LOW);}  //включаем нужные 3
	if(seg[2]!=0){digitalWrite(seg[2]+1,LOW);}
	if(seg[0]!=0){digitalWrite(seg[0]+1,LOW);}
}

void Show_symb(byte symbol_write_1, byte symbol_write_2, byte dot) {
	Lamp(1);
	byte temp = B01000000;
	if (dot == 1)
	{
		digitalWrite(13, LOW);
	}
	else
	{
		digitalWrite(13, HIGH);
	}
	for (short i = 3; i < 10; i++){
		digitalWrite(i, !(symb[symbol_write_1] & temp));
		temp = temp >> 1;
	}
	if (LS_ENABLE) {
		delayMicroseconds(50 * Brightness_conv());
		digitalWrite(MESH1, HIGH);
		delayMicroseconds(50 * (100 - Brightness_conv()));
	}
	else
	{
		delay(5);
	}

	Lamp(2);
	temp = B01000000;
	if (dot == 2)
	{
		digitalWrite(13, LOW);
	}
	else
	{
		digitalWrite(13, HIGH);
	}
	for (short j = 3; j < 10; j++) {
		digitalWrite(j, !(symb[symbol_write_2] & temp));
		temp = temp >> 1;
	}
	if (LS_ENABLE) {
		delayMicroseconds(50 * Brightness_conv());
		digitalWrite(MESH2, HIGH);
		delayMicroseconds(50 * (100 - Brightness_conv()));
	}
	else
	{
		delay(5);
	}
	//Serial.println(Brightness_conv());
}

//void Show_dots(bool show){
//	
//	digitalWrite(13, !show);
//	digitalWrite(0, !show);
//	digitalWrite(1, !show);
//}

short Brightness_conv() {
	short active_time = analogRead(LIGHT_SENSOR);
	if (active_time > 120)return 100;
	if (active_time < 42)return 2;
	return (active_time-40)*1.2;
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
