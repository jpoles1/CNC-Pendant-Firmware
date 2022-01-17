#include <Arduino.h>
#include <TFT_eSPI.h>
#include <jp_tft.h>
#include <BleKeyboard.h>

BleKeyboard keyboard = BleKeyboard("JCNC Pendant");

int rot_pin_A = 26;
int rot_pin_B = 25;
int switch_pin_A = 32;
int switch_pin_B = 33;
int steps = 0;
char current_axis = 'Y';
float move_mult = 1;

unsigned long last_update = 0; // time of last update in ms
static int update_timeout = 250; // time to wait from last update before sending move command to host/cnc

UI ui; //Create TFT UI

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
  if (pin_A && pin_B) current_axis = 'Y';
  else if (pin_A) current_axis = 'X';
  else if (pin_B) current_axis = 'Z';
  /*char log_str[128];
  sprintf(log_str, "Axis: %c", current_axis);
  ui.logtft.add(log_str);*/
}

void setup() {
  pinMode(rot_pin_A, INPUT);
  pinMode(rot_pin_B, INPUT);
  pinMode(switch_pin_A, INPUT_PULLUP);
  pinMode(switch_pin_B, INPUT_PULLUP);
  
  Serial.begin(115200);

  //Start TFT screen
  tft.init();
  tft.setRotation(2);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(1);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  ui.draw();

  //Start rotary encoder
  attachInterrupt(rot_pin_A, rot_interrupt, RISING);
  //attachInterrupt(rot_pin_B, rot_interrupt, RISING);

  //Start rot switch
  attachInterrupt(switch_pin_A, switch_interrupt, RISING);
  attachInterrupt(switch_pin_B, switch_interrupt, RISING);
  switch_interrupt();
  keyboard.begin();
}

void gen_gcode(char* gcode) {
  snprintf(gcode, 100, "G1 %c%.3f", current_axis, steps * move_mult);
}

void handle_as_keyboard(char* gcode) {
  Serial.println(gcode);
  keyboard.println("G91");
  keyboard.println(gcode);
}

void loop() {
  // put your main code here, to run repeatedly:
  if( (millis() - last_update) > update_timeout ) {
    if (steps != 0) {
      char gcode[100];
      gen_gcode(gcode);
      Serial.println("MOVERINO");
      Serial.println(gcode);
      ui.logtft.add(gcode);
      handle_as_keyboard(gcode);
      steps = 0;
    }
    last_update = millis(); // reset timer
  }
  delay(50);
}
