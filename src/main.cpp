#include <JCNC.h>
#include <BleKeyboard.h>

BleKeyboard keyboard = BleKeyboard("JCNC Pendant");

int rot_pin_A = 26;
int rot_pin_B = 25;
int switch_pin_A = 32;
int switch_pin_B = 33;
int top_left_key_pin = 15;
int bot_left_key_pin = 22;
int top_right_key_pin = 17;
int bot_right_key_pin = 21;
int estop_pin = 13;

int steps = 0;

unsigned long last_update = 0; // time of last update in ms
static int update_timeout = 250; // time to wait from last update before sending move command to host/cnc

unsigned long top_left_key_last = 0;
unsigned long bot_left_key_last = 0;
unsigned long top_right_key_last = 0;
unsigned long bot_right_key_last = 0;
unsigned long estop_last = 0;

static int btn_debounce = 250;

JCNC cnc; // Init JCNC

void rot_interrupt() {
	//bool pin_A = digitalRead(rot_pin_A);
	bool pin_B = digitalRead(rot_pin_B);
	//Serial.print("Pin A: ");
	//Serial.println(pin_A);
	//Serial.print("Pin B: ");
	//Serial.println(pin_B);
	steps += pin_B ? 1 : -1;
	last_update = millis();
}

void switch_interrupt() {
	bool pin_A = digitalRead(switch_pin_A);
	bool pin_B = digitalRead(switch_pin_B);
	Serial.println(pin_A);
	Serial.println(pin_B);
	if (pin_A && pin_B) cnc.current_axis = 'Y';
	else if (pin_A) cnc.current_axis = 'X';
	else if (pin_B) cnc.current_axis = 'Z';
	/*char log_str[128];
	sprintf(log_str, "Axis: %c", current_axis);
	ui.logtft.add(log_str);*/
}

void estop_interrupt() {
	cnc.estop = digitalRead(estop_pin);
	Serial.print("Estop change: ");
	Serial.println(cnc.estop);
	steps = 0;
	cnc.draw();
}

void btn_interrupt() {
	bool top_left_key = digitalRead(top_left_key_pin);
	bool bot_left_key = digitalRead(bot_left_key_pin);
	bool top_right_key = digitalRead(top_right_key_pin);
	bool bot_right_key = digitalRead(bot_right_key_pin);

	if(!top_left_key) {
		if ((millis() - top_left_key_last) > btn_debounce) {
			cnc.inc_move_mult();
			Serial.print("Move mult: ");
			Serial.println(cnc.move_mult());
			Serial.println("Top Left Key Pressed");
			cnc.draw();
			top_left_key_last = millis();
		}
	}

	if(!bot_left_key) {
		if ((millis() - bot_left_key_last) > btn_debounce) {
			cnc.dec_move_mult();
			Serial.print("Move mult: ");
			Serial.println(cnc.move_mult());
			Serial.println("Bottom Left Key Pressed");
			cnc.draw();
			bot_left_key_last = millis();
		}
	}

	if(!top_right_key) {
		if ((millis() - top_right_key_last) > btn_debounce) {
			Serial.println("Top Right Key Pressed");
			top_right_key_last = millis();
		}
	}

	if(!bot_right_key) {
		if ((millis() - bot_right_key_last) > btn_debounce) {
			Serial.println("Bottom Right Key Pressed");
			bot_right_key_last = millis();
		}
	}
}

void setup() {
	pinMode(rot_pin_A, INPUT);
	pinMode(rot_pin_B, INPUT);
	pinMode(switch_pin_A, INPUT_PULLUP);
	pinMode(switch_pin_B, INPUT_PULLUP);

	pinMode(top_left_key_pin, INPUT_PULLUP);
	pinMode(bot_left_key_pin, INPUT_PULLUP);
	pinMode(top_right_key_pin, INPUT_PULLUP);
	pinMode(bot_right_key_pin, INPUT_PULLUP);
	pinMode(estop_pin, INPUT_PULLUP);
	
	Serial.begin(115200);

	//Start TFT screen
	cnc.ui.init();
	cnc.draw();

	//Start rotary encoder
	attachInterrupt(rot_pin_A, rot_interrupt, RISING);
	//attachInterrupt(rot_pin_B, rot_interrupt, RISING);

	//Start rot switch
	attachInterrupt(switch_pin_A, switch_interrupt, CHANGE);
	attachInterrupt(switch_pin_B, switch_interrupt, CHANGE);
	switch_interrupt();

	//Start keypad
	attachInterrupt(top_left_key_pin, btn_interrupt, RISING);
	attachInterrupt(bot_left_key_pin, btn_interrupt, RISING);
	attachInterrupt(top_right_key_pin, btn_interrupt, RISING);
	attachInterrupt(bot_right_key_pin, btn_interrupt, RISING);

	attachInterrupt(estop_pin, estop_interrupt, CHANGE);
	estop_interrupt();

	keyboard.begin();
}

void gen_gcode(char* gcode) {
	snprintf(gcode, 100, "G0 %c%.2f", cnc.current_axis, steps * cnc.move_mult());
}

void handle_as_keyboard(char* gcode) {
	Serial.println(gcode);
	keyboard.println("G91");
	keyboard.println(gcode);
}

void loop() {
	if( cnc.estop ) {
		cnc.draw();
		steps = 0;
	} 
	else if( (millis() - last_update) > update_timeout ) {
		if (steps != 0) {
			//switch_interrupt();
			char gcode[100];
			gen_gcode(gcode);
			Serial.println(gcode);
			cnc.ui.logtft.add(gcode);
			handle_as_keyboard(gcode);
			steps = 0;
		}
		last_update = millis(); // reset timer
	}
	delay(50);
}
