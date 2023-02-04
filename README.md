# PAY-PHONE

The pay-phone is a project which combines two devices, a POS machine and a mobile phone and is built with the SEEED XIAO nRF52840 BLE. The program is written in micropython, an efficient version of Python for microcontrollers and embedded devices. This project is currently under development and may contain several bugs and issues and can be recreated only at the users risk.

# Current features
1. 4G/LTE and 2G/GSM connectivity.
2. VoLTE and GSM Calling.
3. UPI based QR payments.
4. SMS messaging (Receiving only)

# Future features
1. Credit/Debit card test payments using Stripe.
2. Control and play music using Spotify.

# Components
1. SEEED XIAO ESP32-C3 or another equivalent ESP32 based microcontroller.
2. SIM 7670CE or another compatible GSM/LTE module.
3. Li-ion battery.
4. Li-ion charging module (can be based on TP4056 or another compatible type).
5. 5V Boost converter.
6. SSD1306 OLED Display (I2C or SPI).
7. 4x4 matrix membrane keypad or another type.
8. PCF8574T Based I2C converter for the keypad.


# Getting started

## Installing Micropython
1. Download Micropython for the SEEED BLE here: https://micropython.org/download/seeed_xiao_nrf52/ if you're using another microcontroller, download Micropython for your board here: https://micropython.org/download/
2. 

## Downloading the editor (Thonny)

## Downloading and installing modules

## Making the connections


# Known issues


# Credits, sources and other documentation

uQR library: https://github.com/JASchilz/uQR

SEEED Documentation: https://wiki.seeedstudio.com/XIAO_BLE/
