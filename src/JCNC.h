#include <TFT_eSPI.h>
#include <list>
#include <Font5x7FixedMono.h>

#define SCREEN_WIDTH 135
#define SCREEN_HEIGHT 240
#define SCREEN_MIDX 67
#define SCREEN_MIDY 120

TFT_eSPI tft = TFT_eSPI(SCREEN_WIDTH, SCREEN_HEIGHT);

class LogTFT {
    public:
        int x, y, w, h, line_height, max_lines;
        LogTFT(int x=0, int y=0, int w=SCREEN_WIDTH, int h=SCREEN_HEIGHT, int font_height=14) {
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
            tft.setFreeFont(&Font5x7FixedMono);
            tft.setTextSize(2);
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

class JCNC {
    class UI {
        public:
            LogTFT logtft = LogTFT(0, 46, SCREEN_WIDTH, SCREEN_HEIGHT - 46);

            void init() {
                tft.init();
                tft.setRotation(2);
                tft.fillScreen(TFT_BLACK);
                tft.setTextColor(TFT_WHITE);
                tft.setTextDatum(MC_DATUM);
                tft.setFreeFont(&Font5x7FixedMono);
                tft.setTextSize(2);
            }

            bool last_flash_phase = 0;

            void draw_estop() {
                uint flash_interval = 1000;
                bool flash_phase = (millis() % flash_interval) > (flash_interval / 2);
                if(flash_phase != last_flash_phase) {
                    if(flash_phase) {
                        tft.fillScreen(TFT_RED);
                        tft.setTextColor(TFT_BLACK);    
                    } else {
                        tft.fillScreen(TFT_BLACK);
                        tft.setTextColor(TFT_RED);
                    }
                    tft.setFreeFont(&Font5x7FixedMono);
                    tft.setTextSize(3);
                    tft.drawString("ESTOP", SCREEN_MIDX, SCREEN_MIDY - 25);
                    tft.drawString("ENGAGE", SCREEN_MIDX, SCREEN_MIDY + 20);
                    last_flash_phase = flash_phase;    
                }
                
            }

            void draw(JCNC *cnc) {
                if (cnc->estop) {
                    draw_estop();
                } else {
                    tft.fillScreen(TFT_BLACK);
                    tft.setTextColor(TFT_WHITE);
                    tft.setTextDatum(MC_DATUM);
                    tft.setFreeFont(&FreeMonoBold9pt7b);
                    tft.setTextSize(1);
                    tft.drawString("JCNC Pendant", SCREEN_MIDX, 4);
                    tft.fillRect(0, 17, SCREEN_WIDTH, 2, TFT_GREEN);
                    char draw_str[20];
                    sprintf(draw_str, "Move x%0.2f", cnc->move_mult());
                    tft.drawString(draw_str, 0, 30);
                    tft.fillRect(0, 44, SCREEN_WIDTH, 2, TFT_GREEN);
                    logtft.draw();
                }
            }
    };

    public: 
        UI ui; //Create TFT UI
        char current_axis = 'Y';
        float move_mult_opts[6] = {0.01, 0.1, 0.5, 1, 5, 10};
        uint move_mult_index = 3;
        bool estop = 0;
    
        float move_mult() {
            return move_mult_opts[move_mult_index];
        }

        void inc_move_mult() {
            uint move_mult_opts_n = sizeof(move_mult_opts)/sizeof(move_mult_opts[0]); //Calc number of option
			move_mult_index = min(move_mult_opts_n - 1, move_mult_index + 1); //Stay in bounds of array
        }

        void dec_move_mult() {
			if (move_mult_index != 0) move_mult_index = move_mult_index - 1; //Can't go lower than 0
        }

        void draw() {
            ui.draw(this);
        }
};