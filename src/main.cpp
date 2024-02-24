#include <JCNC.h>
#include <BluetoothSerial.h>
#include <ESP32Encoder.h>

int dial_pin_A = 26;
int dial_pin_B = 25;
int switch_pin_A = 32;
int switch_pin_B = 33;
int top_left_key_pin = 15;
int bot_left_key_pin = 22;
int top_right_key_pin = 17;
int bot_right_key_pin = 21;
int estop_pin = 13;
int batt_adc_en_pin = 14; //ADC_EN is the ADC detection enable port
int batt_adc_pin = 34; 

int steps = 0;

unsigned long last_update = 0; // time of last update in ms
static int update_timeout = 250; // time to wait from last update before sending move command to host/cnc

unsigned long last_user_input = millis();
static int sleep_timeout = 15 * 1000; // time to wait from last update before sending move command to host/cnc

unsigned long top_left_key_last = 0;
unsigned long bot_left_key_last = 0;
unsigned long top_right_key_last = 0;
unsigned long bot_right_key_last = 0;
unsigned long estop_last = 0;

static int btn_debounce = 250;

JCNC cnc; // Init JCNC
BluetoothSerial SerialBT;
ESP32Encoder dial;

uint8_t bt_address[6]  = {0x00, 0x18, 0xE4, 0x40, 0x00, 0x06};
const char *bt_pin = "4424";


/*void handle_as_keyboard(char* gcode) {
	if(!keyboard.isConnected()) {
		cnc.ui.logtft.add("BLE Discon");
	}
	Serial.println(gcode);
	keyboard.println("G91");
	keyboard.println(gcode);
}*/

float read_batt_voltage() {
	const int vref = 1100;
	uint16_t v = analogRead(batt_adc_pin);
	float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
	return battery_voltage;
}

void handle_as_ble_grbl(char* gcode) {
	/*if(!SerialBT.connected()) {
		cnc.ui.logtft.add("BLE Discon");
	}*/
	SerialBT.println("G91");
	SerialBT.println(gcode);
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
	last_user_input = millis();
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
			cnc.ui.handle_top_left_key();
			Serial.println("Top Left Key Pressed");
			top_left_key_last = millis();
		}
	}

	if(!bot_left_key) {
		if ((millis() - bot_left_key_last) > btn_debounce) {
			cnc.ui.handle_bot_left_key();
			Serial.println("Bottom Left Key Pressed");
			bot_left_key_last = millis();
		}
	}

	if(!top_right_key) {
		if ((millis() - top_right_key_last) > btn_debounce) {
			cnc.ui.handle_top_right_key();
			Serial.println("Top Right Key Pressed");
			top_right_key_last = millis();
		}
	}

	if(!bot_right_key) {
		if ((millis() - bot_right_key_last) > btn_debounce) {
			handle_as_ble_grbl("$H");
			Serial.println("Bottom Right Key Pressed");
			bot_right_key_last = millis();
		}
	}
}

void bt_watchdog(void * params) {
	bool last_conn_state = 0;
	for(;;) {
		bool connected = SerialBT.connected(1000);
		/*Serial.print("CONN STATUS: ");
		Serial.println(connected);*/
		if(!connected) {
			if(last_conn_state){
				cnc.ui.logtft.add("BT DISCONN");
				last_conn_state = 0;
			}
			SerialBT.disconnect();
			SerialBT.connect(bt_address);
			bool connnect_attept = SerialBT.connected(10000);
			if(connnect_attept) {
				cnc.ui.logtft.add("BT CONN");
				Serial.println("Connected Succesfully!");
				last_conn_state = 1;
			} else {
				cnc.ui.logtft.add("BT CONN ERR");
				Serial.println("Failed to connect!");
			}
		}
		vTaskDelay(2.5 * 1000 / portTICK_PERIOD_MS);
	}
}

void setup() {
	Serial.begin(115200);
	
	ESP32Encoder::useInternalWeakPullResistors=DOWN;
	dial.attachSingleEdge(dial_pin_A, dial_pin_B);

	pinMode(switch_pin_A, INPUT_PULLUP);
	pinMode(switch_pin_B, INPUT_PULLUP);

	pinMode(top_left_key_pin, INPUT_PULLUP);
	pinMode(bot_left_key_pin, INPUT_PULLUP);
	pinMode(top_right_key_pin, INPUT_PULLUP);
	pinMode(bot_right_key_pin, INPUT_PULLUP);
	pinMode(estop_pin, INPUT_PULLUP);
	/*
    ADC_EN is the ADC detection enable port
    If the USB port is used for power supply, it is turned on by default.
    If it is powered by battery, it needs to be set to high level
    */
    pinMode(batt_adc_en_pin, OUTPUT);
    digitalWrite(batt_adc_en_pin, HIGH);

	//Start TFT screen
	cnc.ui.init();
	cnc.draw();

	//remove_paired();

	//Start BLE Serial
	if(!SerialBT.begin("JCNC Pendant", true)) {
		Serial.println("An error occurred initializing Bluetooth");
		cnc.ui.logtft.add("BT INIT ERR");
	};
	cnc.ui.logtft.add("BT START");
	
	SerialBT.setPin(bt_pin);
	
	xTaskCreate(bt_watchdog, "bt_watchdog", 10000, NULL, 1, NULL);

	esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 0); //1 = High, 0 = Low
	
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
}

void gen_gcode(char* gcode, float steps) {
	snprintf(gcode, 100, "G0 %c%.2f", cnc.current_axis, steps * cnc.move_mult());
}



void loop() {
	if( cnc.estop ) {
		cnc.draw();
		dial.clearCount();
		if(millis() - last_user_input > sleep_timeout) {
			esp_deep_sleep_start();
		}
	} 
	else if( (millis() - last_update) > update_timeout ) {
		while (SerialBT.available()) {
			Serial.write(SerialBT.read());
		}
		int steps = dial.getCount();
		dial.clearCount();
		if (steps != 0) {
			//switch_interrupt();
			char gcode[100];
			gen_gcode(gcode, steps);
			Serial.println(gcode);
			cnc.ui.logtft.add(gcode);
			handle_as_ble_grbl(gcode);
			last_user_input = millis();
		}
		Serial.println(read_batt_voltage());
		last_update = millis(); // reset timer
	}
	delay(50);
}
