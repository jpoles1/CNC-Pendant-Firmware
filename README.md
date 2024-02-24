## CNC Pendant

This repo houses the fimrware for a 3d printed ESP32 CNC pendant which is used to more easily control CNC machines. 

For the full Bill of Materials and 3D Printer files, see here: https://www.printables.com/model/778491-diy-cnc-pendant

The pendant has two modes:
- Bluetooth: Communicates with a basic CNC controller board (like those which often come with a CNC 3018) using a Bluetooth HC-05 which directly sends GCode commands via serial
- WiFi: Utilizes the bCNC web pendant to receive HTTP requests containing GCode commands.

### Setup:
1) Clone this repo and open it using [PlatformIO](https://platformio.org/platformio-ide) (I use the VSCode extension)
1) Switch between modes using `#define CONMODE` in `main.cpp` setting it to either BT_MODE or WIFI_MODE (default: WIFI_MODE).
2) Copy secrets.example to secrets.h and fill in your own values (`cp secrets.example secrets.h`).
3) Upload to your ESP32 board
4) If using the bCNC WiFi mode, make sure the pendant server is up and running
5) CNC your heart out!

