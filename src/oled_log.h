#include <Adafruit_SSD1306.h>
#include <Wire.h>

#include <bits/stdc++.h>
#include <iostream>
#include <sstream>
#include <string>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

std::vector<std::string> splitString(std::string input, unsigned string_size = 10) {
    std::vector<std::string> split_string;
    for (unsigned i = 0; i < input.length(); i += string_size) {
        split_string.push_back(input.substr(i, string_size));
    }
    return split_string;
}

void oledString(std::string oled_data, bool newline = true, bool clearScreen = false) {
    std::vector<std::string> split_string = splitString(oled_data, 21);
    for (std::vector<std::string>::iterator it = split_string.begin(); it != split_string.end(); ++it) {
        oled.clearDisplay();
        std::string txt = *it;
        char *cstr = new char[txt.length() + 1];
        strcpy(cstr, txt.c_str());
        if (newline) {
            Serial.println(cstr);
            oled.println(cstr);
        } else {
            Serial.print(cstr);
            oled.print(cstr);
        }
        delete[] cstr;
        // oled it on the screen
        oled.display();
    }
    // Print to the screen
    if (newline) {
        delay(50);
    }
}

void oledInt(int oled_data, bool newline = false, bool clearScreen = false) {
    std::ostringstream ss;
    ss.str("");
    ss << oled_data;
    oledString(ss.str(), newline, clearScreen);
}

void oledStringVector(std::vector<std::string> oled_data,
                         bool clearScreen = false) {
    if (clearScreen == true) {
        oled.setLogBuffer(5, 30);
    }
    for (std::vector<std::string>::iterator it = oled_data.begin();
            it != oled_data.end(); ++it) {
        // Print to the screen
        oledString(*it, true, clearScreen);
    }
}