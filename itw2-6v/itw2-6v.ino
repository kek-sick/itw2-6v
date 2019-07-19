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

unsigned long  
	fst_tap_time = 0, 
	GetTemp_time = 0, 
	mills_ftt = 0, 
	timeSet_mode_init_time = 0,
	timeSet_mode_time = 0;
byte a = 0,temperature = 0;
byte time_set_btn = 0,
	last_time_set_btn = 0,
	current_min = 0,
	current_hour = 0;

bool first_tap_btn_flag = false,	//if one tap happened
	second_tap_btn_flag = false,	//if two taps hepptned
	allow_secon_tap = false,		//if TRUE then second tap can be detected
	mercury_btn_flag = false,		//TRUE if btn parcer detected high value
	ignition = false,				//TRUE when power lines are active
	has_timeSet_shown = false,
	has_shown = false;				//TRUE when showing func has done
volatile short btn_type = 0;
volatile bool time_set_btn_flag = false, btn_flag = false;
	
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
	if (btn_type > 700)
	{
		time_set_btn_flag = false;
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
	
	if (btn_flag)
	{
		//Serial.println(analogRead(BTN_DETECT));
		if (btn_type > 610 && btn_type < 660)mercury_btn_flag = true;
		if (btn_type > 770 && btn_type < 810)time_set_btn = 1;	//hours
		if (btn_type > 950)time_set_btn = 2;					//minutes
	}
	else
	{
		//time_set_btn = 0;
		mercury_btn_flag = false;
	}

	timeSet_mode_time = millis() - timeSet_mode_init_time;

	if (time_set_btn > 0 && !time_set_btn_flag)		//restart time set mod timer on every button hit
	{
		time_set_btn_flag = true;
		has_timeSet_shown = false;
		if (timeSet_mode_time < 6400){
			timeSet_mode_init_time = millis() - 600;
		}
		else {
			timeSet_mode_init_time = millis();
		}
		if (time_set_btn == last_time_set_btn)		//incrementing time
		{
			if (time_set_btn == 1) { current_hour++; }
			else { current_min++; }
			if (current_hour == 24)current_hour = 0;
			if (current_min == 60)current_min = 0;
		}
		if (time_set_btn != last_time_set_btn)		//changing the displayimg time
		{
			clock.begin();							//getting time
			clock.settime(0, current_min, current_hour);				//saving time				
			current_hour = clock.Hours;
			current_min = clock.minutes;
		}

		last_time_set_btn = time_set_btn;			//setting last time button
		
	}

	/*Serial.println(timeSet_mode_time);
	Serial.println(time_set_btn);
	Serial.println(ignition);
	Serial.println(has_timeSet_shown);
	Serial.println("----");*/

	if (time_set_btn != 0 && timeSet_mode_time >= 6400 && !has_timeSet_shown)		//saving time
	{
		clock.begin();
		clock.settime(0, current_min, current_hour);
		time_set_btn = 0;
		last_time_set_btn = 0;
		has_timeSet_shown = true;
	}

	if (time_set_btn != 0 && timeSet_mode_time > WF_SECOND_TAP && timeSet_mode_time < 6400)
	{
		if (time_set_btn == 1)
		{
			Show_timeSet(current_hour, time_set_btn);
		}
		else
		{
			Show_timeSet(current_min, time_set_btn);
		}
	}

	if (mills_ftt < WF_SECOND_TAP)   //показываем анимацию
	{
		if (mills_ftt % 10 < 5)
		{
			Show_anim(mills_ftt / 60 - 1, 1, ANIMATION);
		}
		else 
		{
			Show_anim(mills_ftt / 60 - 1, 2, ANIMATION);
		}
	}

	if (timeSet_mode_time < WF_SECOND_TAP)
	{
		if (timeSet_mode_time % 10 < 5)
		{
			Show_anim(timeSet_mode_time / 60 - 1, 1, ANIMATION);
		}
		else
		{
			Show_anim(timeSet_mode_time / 60 - 1, 2, ANIMATION);
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
	if ((time_set_btn != 0 || first_tap_btn_flag) && !ignition)
	{
		ignition = true;
		digitalWrite(FILAMENT, HIGH);
		digitalWrite(HIGH_VOLTAGE, HIGH);
	}
	//showing time
	if (first_tap_btn_flag && !second_tap_btn_flag && !has_shown && mills_ftt > WF_SECOND_TAP)
	{
		Show_time();
		//first_tap_btn_flag = false;
		has_shown = true;
		//allow_secon_tap = false;
	}
	//showing data
	if (first_tap_btn_flag && second_tap_btn_flag && !has_shown && mills_ftt > WF_SECOND_TAP)
	{
		Show_temperature();
		
		has_shown = true;
		
		second_tap_btn_flag = false;
	}
	//showing closing animation
	if ((has_shown && mills_ftt > 2800 && mills_ftt < 3400) || (timeSet_mode_time > 6400 && timeSet_mode_time < 7000 && has_timeSet_shown))
	{
		if (has_timeSet_shown)
		{
			if (timeSet_mode_time % 10 < 5)
			{
				Show_anim(8 - (timeSet_mode_time - 6400) / 60, 1, ANIMATION);
			}
			else
			{
				Show_anim(8 - (timeSet_mode_time - 6400) / 60, 2, ANIMATION);
			}
		}
		else
		{
			if (mills_ftt % 10 < 5)
			{
				Show_anim(8 - (mills_ftt - 2800) / 60, 1, ANIMATION);
			}
			else
			{
				Show_anim(8 - (mills_ftt - 2800) / 60, 2, ANIMATION);
			}
		}
		
		//Serial.println(mills_ftt);
	}
	//turning off power
	if (ignition && (has_shown && mills_ftt > 3400 || timeSet_mode_time > 7000 && has_timeSet_shown))
	{
		allow_secon_tap = false;
		first_tap_btn_flag = false;

		has_shown = false;
		has_timeSet_shown = false;
		ignition = false;
		digitalWrite(HIGH_VOLTAGE, LOW);
		digitalWrite(FILAMENT, LOW);
		delay(30);
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

void Show_timeSet(byte time, byte time_type) {
	byte time1, time2;
	time1 = time / 10;
	time2 = time % 10;
	if (timeSet_mode_time % 1000 < 500)
	{
		Show_symb(time1, time2, 3 - time_type);
	}
	else
	{
		Show_symb(time1, time2, 0);
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
	//Show_symb(16, 16 , 0);
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
	tt1 = clock.minutes / 10;
	tt2 = clock.minutes % 10;
	Show_symb(16, 16 , 0);
	//delay(300);
	while (millis() - fst_tap_time < 2800)
	{
		Show_symb(tt1, tt2, 1);
	}
	//Show_symb(16, 16, 0);
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
	digitalWrite(13, HIGH);
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
	}	else	{
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
	}	else	{
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
