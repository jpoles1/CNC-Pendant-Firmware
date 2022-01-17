#include <TFT_eSPI.h>
#include <list>

#define SCREEN_WIDTH 135
#define SCREEN_HEIGHT 240
#define SCREEN_MIDX 67
#define SCREEN_MIDY 120

TFT_eSPI tft = TFT_eSPI(SCREEN_WIDTH, SCREEN_HEIGHT);

class LogTFT {
    public:
        int x, y, w, h, line_height, max_lines;
        LogTFT(int x=0, int y=0, int w=SCREEN_WIDTH, int h=SCREEN_HEIGHT, int font_height=12) {
            this->x = x;
            this->y = y;
            this->w = w;
            this->h= h;
            this->line_height = font_height + 2;
            this->max_lines = floor(h / this->line_height) - 2;
        }
        void add(char* msg) {
            char *msg_copy = strdup(msg);
            if(log.size() == max_lines) {
                Serial.println(log.front());
                Serial.println(log.back());
                char* tbd = log.back();
                log.pop_back();
                free(tbd);
            }
            log.push_front(msg_copy);
            draw();
        }
        void draw() {
            tft.fillRect(x, y, w, h, TFT_BLACK);
            int line_n = 1;
            for(std::list<char*>::iterator log_entry = log.begin(); log_entry != log.end(); ++log_entry) {
                if (line_n %2 == 0) {
                    tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
                } else {
                    tft.setTextColor(TFT_WHITE, TFT_BLACK);
                }
                tft.drawString(*log_entry, this->x, this->y + (line_height) * line_n);
                line_n++;
            }
        }
    private:
        std::list<char*> log;
};

class UI {
    public:
        LogTFT logtft = LogTFT(0, 22, SCREEN_WIDTH, SCREEN_HEIGHT - 22);
        UI() {
            logtft.add("STARTING...");
        }
        void draw() {
            tft.setTextColor(TFT_WHITE);
            tft.setTextDatum(MC_DATUM);
            tft.drawString("JCNC Pendant", SCREEN_WIDTH/2, 4);
            tft.fillRect(0, 17, SCREEN_WIDTH, 2, TFT_GREEN);
            logtft.draw();
        }
};