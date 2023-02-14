# ESP-PHONE

ESP-Phone is a project which combines two devices, a POS machine and a basic mobile phone and is built with the SEEED XIAO ESP32-C3. The program is written in C++. This project is currently under development and may contain several bugs and issues.

# Current features
1. 4G/LTE and 2G/GSM connectivity.
2. VoLTE and GSM Calling.
3. UPI based QR payments.
4. SMS messaging (Receiving only).

# Future features
1. Credit/Debit card test payments using Stripe.
2. Control and play music using Spotify.

# Components
1. SEEED XIAO ESP32-C3 or another equivalent ESP32 based microcontroller.
2. SIM 7670CE or another compatible GSM/LTE module.
3. Li-ion battery.
4. Li-ion charging module (can be based on TP4056 or another compatible type).
5. 5V Boost converter.
6. SSD1306 OLED Display (I2C).
7. 4x4 matrix membrane keypad or another type.
8. PCF8574T Based I2C controller for the keypad.


# Getting started

## Installing Arduino IDE
1. Download the Arduino IDE here: https://www.arduino.cc/en/software

## Installing the board to the Arduino IDE
1. Download and install the SEED XIAO ESP32-C3 board to the Arduino IDE following this tutorial: https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/

## Downloading and installing required libraries
Download and install all the libraries by navigating to: `sketch > Include Library > Manage Libraies... ` Then search for and intall the following libraries:
1. `Wire.h` (Should be pre-installed)
2. `HardwareSerial.h` (Should be pre-installed)
3. `Keypad.h` and `Keypad_I2C.h` refer: https://playground.arduino.cc/Code/Keypad/
4. `SSD1306Wire.h` refer: https://github.com/ThingPulse/esp8266-oled-ssd1306
5. `qrcodeoled.h` refer: https://github.com/yoprogramo/QRcodeOled and https://github.com/yoprogramo/QRcodeDisplay

## Making the connections
1. Connect the OLED display to power 5V and GND and the SDA and SCL pins to the I2C bus on the microcontroller. (SDA on the XIAO is pin 4 and SCL is pin 5)
2. Connect the Keypad to the same I2C bus and the VCC pin to 5V and GND to GND. SDA->SDA and SCL->SCL.
3. Connect the GSM module to 5V power. GSM TX to ESP RX and GSM RX to ESP TX. On my GSM module TX and RX pins are lablled incorrectly, if this is the case with the module you use, make sure to connect them correctly. (TX of the XIAO is pin 6 and RX is pin 7)
4. Connect any external antenna to the GSM module.
5. Connect a microphone to the GSM module MIC (M+ and M-) pins and a speaker to (S+ and S-) pins. The module usually includes an amplifier, remember to read the documentation for speaker and mic types for best results and to avoid damage.
6. Connect the included antenna to the ESP32-C3.

# Issues
Please refer to the issues section for this repository for all issues or to submit a new issue.


# Credits, sources and other documentation
1. SEEED Documentation: https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/


# Purchase Links
1. SEEED Xiao ESP32-C3: https://www.seeedstudio.com/Seeed-XIAO-ESP32C3-p-5431.html
