#include <Arduino.h>
#include <Wire.h>

int rot_pin_A = 26;
int rot_pin_B = 25;
int steps = 0;

unsigned long last_update = 0; // time of last update in ms
static int update_timeout = 250; // time to wait from last update before sending move command to host/cnc

void rot_interrupt() {
  //bool pin_A = digitalRead(rot_pin_A);
  bool pin_B = digitalRead(rot_pin_B);
  //Serial.print("Pin A: ");
  //Serial.println(pin_A);
  //Serial.print("Pin B: ");
  //Serial.println(pin_B);
  steps += pin_B ? -1 : 1;
  last_update = millis();
}

void setup() {
  pinMode(rot_pin_A, INPUT);
  pinMode(rot_pin_B, INPUT);
  Serial.begin(115200);
  Wire.begin(5, 4);
    if(!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000); // Pause for 2 seconds
 
  // Clear the buffer.
  oled.setRotation(1);
  oled.clearDisplay();
  oled.setTextColor(WHITE);
  oled.setTextSize(1);
  oled.println("STARTING...");
  oled.display();
  attachInterrupt(rot_pin_A, rot_interrupt, RISING);
  //attachInterrupt(rot_pin_B, rot_interrupt, RISING);
}

void loop() {
  // put your main code here, to run repeatedly:
  if( (millis() - last_update) > update_timeout ) {
    if (steps != 0) {
      Serial.print("Move: ");
      Serial.println(steps);
      oled.clearDisplay();
      oled.setCursor(0,28);
      oled.print("Move: ");
      oled.println(steps);
      oled.display();
      steps = 0;
    }
    last_update = millis(); // reset timer
  }
  delay(50);
}

void handle_as_keyboard(command) {

}