/*
  * AUTHOR: @adityad0 [https://github.com/adityad0/]
  * PAY-PHONE https://github.com/adityad0/pay-phone/
  * LICENSE: https://github.com/adityad0/pay-phone/LICENSE.md
  * A POS payment system built with the ESP32 with support for NFC, QR, Bluetooth and phone call support.
*/
#include <Wire.h>
#include "SSD1306Wire.h"
#include <Keypad.h>
#include <Keypad_I2C.h>
#include <qrcodeoled.h>
#include <HardwareSerial.h>

// Global variables
#define SERIAL_BAUD 115200
#define GSM_BAUD 9600
char gsm_resp[100] = "";
char gsm_operator[100] = "Unknown";
float net_strength = 0.00;
char call_phone_number[100] = "";

// OLED Initialization
#define OLED_ADDR 0x3C
SSD1306Wire display(OLED_ADDR, SDA, SCL);
QRcodeOled qrcode(&display);

// Keypad Initialization
#define KEYPAD_ADDR 0x20
const byte KEYPAD_ROWS = 4;
const byte KEYPAD_COLS = 4;
char keys [KEYPAD_ROWS] [KEYPAD_COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins [KEYPAD_ROWS] = {7, 6, 5, 4};
byte colPins [KEYPAD_COLS] = {3, 2, 1, 0};
Keypad_I2C keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS, KEYPAD_ADDR, PCF8574);

// GSM Serial Initialization
HardwareSerial gsm(0);

void setup() {
  // Setup serial
  Serial.begin(115200);
  Serial.println("Serial initialized.");

  // Setup display
  display.init();
  display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Display initialized.");
  Serial.println("Display initialized.");
  display.display();
  delay(1000);

  // Setup QR code library
  qrcode.init();
  qrcode.create("https://github.com/adityad0/pay-phone/");
  Serial.println("QR initialized.");

  // Setup keypad
  Wire.begin();
  keypad.begin(makeKeymap(keys));
  Serial.println("Keypad initialized.");
  delay(1000);

  // Setup GSM communication
  gsm.begin(GSM_BAUD, SERIAL_8N1, -1, -1);
  Serial.println("GSM initialized.");
  delay(1000);

  // Get the GSM operator and network strength
  String net_operator;
  net_operator = get_gsm_operator();
  char net_strength = get_net_strength();

  // Display the operator
  display.clear();
  display.drawString(0, 0, net_operator);
  display_menu();
}

void loop () {
  // Keep looking for a keypress on the keypad, then print the key to the serial terminal
  char key = keypad.getKey();
  if(key) {
    Serial.println(key);
    display.clear();
    display.drawString(0, 0, &key);
    display.display();
  }
}

// This function resets all the characters of any given array to '\0'
void reset_array(char *array) {
  memset(array, '\0', sizeof(array));
}

// This function will return a character array of gsm_resp_array - gsm_command. This will remove the message sent to the module and will return only the response from the module.
char parse_gsmResponse(char *gsm_resp_array, char *gsm_command) {
  char *ptr = strstr(gsm_resp_array, gsm_command);
  if(ptr != NULL) {
    ptr += strlen(gsm_command);
    return *ptr;
  } else {
    return '\0';
  }
}

// This function waits untill a keypress and return the key which is pressed.
char getKeyPress() {
  while(1) {
    char key = keypad.getKey();
    if(key) {
      Serial.print("Key pressed: ");
      Serial.println(key);
      return key;
    }
  }
}

void display_menu() {
  display.drawString(0, 10, "1. Call");
  display.drawString(0, 20, "2. View SMS");
  display.drawString(0, 30, "3. UPI QR");
  display.drawString(0, 40, "4. Settings");
  display.display();
  char key = getKeyPress();
  if(key == '1') {
    make_call();
  } else if(key == '2') {
    view_sms();
  } else if(key == '3') {
    Serial.println("UPI QR.");
    display.clear();
    display_upi_qr("test@test", "1", "Test");
    getKeyPress();
    display.clear();
    display_menu();
  } else if(key == '4') {
    Serial.println("Settings.");
    display.clear();
    settings_menu();
  } else {
    Serial.println("Invalid option presses on menu.");
    display.drawString(0, 50, "Invalid option!");
    display.display();
    delay(1000);
    display.clear();
    display_menu();
  }
}

int send_at_command(const char* cmd, char* gsm_resp) {
  reset_array(gsm_resp);
  char *ptr = gsm_resp;
  gsm.println(cmd);
  delay(500);
  while(gsm.available()) {
    *ptr = gsm.read();
    ptr++;
  }
  *ptr = '\0';
  return ptr - gsm_resp;
}

char get_gsm_operator() {
  reset_array(gsm_resp);
  int at_resp_len = send_at_command("AT+COPS=3,0", gsm_resp);
  reset_array(gsm_resp);
  at_resp_len = send_at_command("AT+COPS?", gsm_resp);
  Serial.print("Network operator: ");
  Serial.println(gsm_resp);
  return *gsm_resp;
}

char get_net_strength() {
  reset_array(gsm_resp);
  int at_resp_len = send_at_command("AT+CSQ", gsm_resp);
  Serial.print("Network strength: ");
  Serial.println(gsm_resp);
  return *gsm_resp;
}

void make_call() {
  display.clear();
  display.drawString(0, 0, "MAKE CALL");
  display.drawString(0, 10, "Number:");
  display.display();
  char call_keypad_handler_ret = call_keypad_handler(call_phone_number);
  if(call_keypad_handler_ret == 'D') {
    Serial.print("Dialing: ");
    Serial.println(call_phone_number);
    send_at_command("ATD9538271867;", gsm_resp);
    Serial.println(gsm_resp);
    display.clear();
    display.drawString(0, 0, "Dialing:");
    display.drawString(0, 10, call_phone_number);
    display.drawString(0, 20, gsm_resp);
    display.display();
    delay(5000);
    display.clear();
    display_menu();
  }
  delay(2000);
  display.clear();
}

char call_keypad_handler(char* call_phone_number) {
  char *ptr = call_phone_number;
  while(1) {
    char key = keypad.getKey();
    if(key) {
      if(key == 'A') {
        // Cancel
        Serial.println("Phone call dial cancelled.");
        ptr = call_phone_number;
        *ptr = '\0';
        reset_array(call_phone_number);
        display.clear();
        display_menu();
      } else if(key == 'B') {
        // Backspace
        Serial.println("Backspace");
        if(ptr != call_phone_number) {
          ptr--;
          *ptr = '\0';
          display.clear();
          display.drawString(0, 0, "MAKE CALL");
          display.drawString(0, 10, "Number:");
          display.drawString(0, 20, call_phone_number);
          display.display();
        }
      } else if(key == 'C') {
        // Clear the phone number
        reset_array(call_phone_number);
        Serial.println("Phone number cleared.");
        display.clear();
        display.drawString(0, 0, "MAKE CALL");
        display.drawString(0, 10, "Number:");
        display.display();
      } else if(key == 'D') {
        // Make a call here
        Serial.println("D");
        *ptr = '\0';
        return 'D';
      } else if(key == '#' || key == '*') {
        continue;
      } else {
        *ptr = key;
        ptr++;
        Serial.print("Phone number entered: ");
        Serial.println(call_phone_number);
        display.drawString(0, 20, call_phone_number);
        display.display();
      }
    }
  }
}

void view_sms() {
  display.clear();
  display.drawString(0, 0, "View SMS.");
  display.display();
  delay(1000);
  display.clear();
  display_menu();
}

void display_upi_qr(String upi_id, String amount, String display_name) {
  display.clear();
  String upi_url = "upi://pay?pa=" + upi_id + "&pn=" + display_name + "&cu=INR&am=" + amount;
  qrcode.create(upi_url);
}

void settings_menu() {
  display.drawString(0, 0, "SETTINGS");
  display.drawString(0, 10, "1. Sleep");
  display.drawString(0, 20, "2. Power off");
  display.drawString(0, 30, "3. Device info");
  display.drawString(0, 40, "4. Menu");
  display.display();

  char key = getKeyPress();
  if(key == '1') {
    Serial.println("Sleep mode requested.");
    display.clear();
    display.drawString(0, 0, "Enter sleep mode (0/1)?");
    display.display();
    char key = getKeyPress();
    if(key == '1') {
      Serial.println("Sleep mode confirmed.");
      display.clear();
      Serial.println("Sleep mode initiated.");
      display.drawString(0, 0, "Device going to sleep..\nPress any key to wake up..");
      display.display();
      send_at_command("AT+CSCLK=1", gsm_resp);
      delay(5000);
      display.clear();
      display.display();
      Serial.println("Sleep mode enabled.");
      getKeyPress();
      send_at_command("AT", gsm_resp);
      display.clear();
      display_menu();
    } else {
      Serial.println("Sleep mode aborted.");
      display.clear();
      settings_menu();
    }
  } else if(key == '2') {
    Serial.println("TODO: Power off.");
    display.clear();
    display.drawString(0, 0, "TODO: Power off.");
    display.display();
    delay(1000);
    display.clear();
    settings_menu();
  } else if(key == '3') {
    Serial.println("Device info.");
    display.clear();
    display.drawString(0, 0, "INFO");
    display.drawString(0, 10, "Operator: Unknown");
    display.drawString(0, 20, "Strength: 0.00");
    display.display();
    display.clear();
    getKeyPress();
    settings_menu();
  } else if(key == '4' || key == 'A') {
    Serial.println("Menu.");
    display.clear();
    display.display();
    display_menu();
  } else {
    Serial.println("Invalid option presses on settings menu.");
    display.drawString(0, 50, "Invalid option!");
    display.display();
    settings_menu();
  }
}
