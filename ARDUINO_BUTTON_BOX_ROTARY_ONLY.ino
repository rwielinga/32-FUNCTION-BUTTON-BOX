//BUTTON BOX 
//USE w ProMicro
//simplified for single rotary from AMSTUDIO 20.8.17

#include <Joystick.h>

struct rotarydef {
  byte pin1; // Arduino pin
  byte pin2; // Arduino pin
  int ccwchar; // Send to PC
  int cwchar; // Send to PC
  volatile unsigned char state; // Internal state
};

rotarydef rotary {0,1,24,25,0};

#define DIR_CCW			0x10
#define DIR_CW			0x20

//State rows:
#define R_START			0x0
#define R_CW_FINAL	0x1
#define R_CW_BEGIN	0x2
#define R_CW_NEXT		0x3
#define R_CCW_BEGIN	0x4
#define R_CCW_FINAL	0x5
#define R_CCW_NEXT	0x6
// seems a bit like a magical table... what does this do?
// state table: given starting state lookup resulting state given an action
// is state is first column, 
const unsigned char ttable[7][4] = {
					{R_START,    R_CW_BEGIN,  R_CCW_BEGIN, R_START},						// R_START
					{R_CW_NEXT,  R_START,     R_CW_FINAL,  R_START | DIR_CW},		// R_CW_FINAL
					{R_CW_NEXT,  R_CW_BEGIN,  R_START,     R_START},						// R_CW_BEGIN
					{R_CW_NEXT,  R_CW_BEGIN,  R_CW_FINAL,  R_START},						// R_CW_NEXT
					{R_CCW_NEXT, R_START,     R_CCW_BEGIN, R_START},						// R_CCW_BEGIN
					{R_CCW_NEXT, R_CCW_FINAL, R_START,     R_START | DIR_CCW},	// R_CCW_FINAL
					{R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START},						// R_CCW_NEXT
};

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_JOYSTICK, 32, 0,
  false, false, false, false, false, false, false, false, false, false, false);

void setup() {
  Joystick.begin();
  rotary_init();
  Serial.begin(9600);
}

void loop() { 
	unsigned char result = rotary_process();
	if (result == DIR_CCW) { Joystick.setButton(rotary.ccwchar, 1); delay(50); Joystick.setButton(rotary.ccwchar, 0); };
	if (result == DIR_CW) { Joystick.setButton(rotary.cwchar, 1); delay(50); Joystick.setButton(rotary.cwchar, 0); };
}

void rotary_init() {
    pinMode(rotary.pin1, INPUT);
    pinMode(rotary.pin2, INPUT);
		digitalWrite(rotary.pin1, HIGH);
		digitalWrite(rotary.pin2, HIGH);
}

unsigned char prev_pinstate = 0;

unsigned char rotary_process() {
	// Determine rotation action
	unsigned char pinstate = (digitalRead(rotary.pin2) << 1) | digitalRead(rotary.pin1);
  // should be 0x20 for CW or 0x10 for CCW or 00000000 for no action i guess. Both pins high should not happen
  if (prev_pinstate != pinstate) {
    Serial.println("New pinstate: " + pinstate); // print the pinstate if it changed
    prev_pinstate = pinstate;
  }
	volatile unsigned char lookedupstate = ttable[rotary.state & 0xf][pinstate];
	if (rotary.state != lookedupstate) {
    Serial.println("New rotarystate: " + lookedupstate); // print the lookedupstate for the rotary if changed
    rotary.state = lookedupstate;
  }
	return (rotary.state & 0x30);
}
