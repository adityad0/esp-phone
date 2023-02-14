/*
  * AUTHOR: @adityad0 [https://github.com/adityad0/]
  * ESP-PHONE https://github.com/adityad0/esp-phone/
  * LICENSE: https://github.com/adityad0/esp-phone/LICENSE.md
  * A home made POS payment system and mobile phone built with the SEED XIAO ESP32-C3 with support for BLE, Wi-Fi and 4G/LTE support.
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
char gsm_operator[50] = "Unknown";
char net_strength[20] = "0.00";
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

  // Use core 1 for display, user interface and keypad
  Serial.print("Running on core: ");
  Serial.println(xPortGetCoreID());

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
  get_gsm_operator();
  get_net_strength();

  Serial.print("Parsed GSM operator: ");
  Serial.println(gsm_operator);

  // Use core 0 to keep checking for incoming calls and SMS
  // Create a new task on core for handling incoming calls and SMS
  xTaskCreatePinnedToCore(
    handle_incoming_calls,   // Function that should be called
    "handle_incoming_calls", // Name of the task (for debugging)
    10000,                   // Stack size (bytes)
    NULL,                    // Input parameter to pass
    1,                       // Task priority
    NULL,                    // Task handle
    0                        // Core where the task should run
  );

  // Call the main menu function
  display.clear();
  display_menu();
}

void loop () {
}

// Function that runs on core 0 to handle incoming calls and SMS
void handle_incoming_calls(void *parameter) {
  Serial.print("handle_incoming_calls task started on core: ");
  Serial.println(xPortGetCoreID());
  while(1) {
    if(gsm.available()) {
      char c = gsm.read();
      Serial.print(c);
      if(c == '\n') {
        Serial.println("New line detected from GSM.");
        if(strstr(gsm_resp, "RING") != NULL) {
          Serial.println("Incoming call detected.");
          display.clear();
          display.drawString(0, 0, "Incoming call");
          display.drawString(0, 10, "CXL: Reject");
          display.drawString(0, 20, "ACP: Answer");
          display.display();
          while(1) {
            char key = keypad.getKey();
            if(key == 'A') {
              Serial.println("Call rejected.");
              gsm.println("ATH;");
              display.clear();
              display.drawString(0, 0, "Call rejected.");
              display.drawString(0, 10, "Press any key for menu.");
              display.display();
              delay(2000);
              break;
            } else if(key == 'D') {
              gsm.println("ATA;");
              Serial.println("Call accepted.");
              display.clear();
              display.drawString(0, 0, "Ongoing call..");
              display.drawString(0, 10, "CXL: End call");
              display.display();
              while(1) {
                char key = keypad.getKey();
                if(key == 'A') {
                  gsm.println("ATH;");
                  Serial.println("Incoming call Ended.");
                  display.clear();
                  display.drawString(0, 0, "Call ended.");
                  display.display();
                  delay(2000);
                  break;
                }
              }
              break;
            }
          }
        }
        reset_array(gsm_resp);
      } else {
        strncat(gsm_resp, &c, 1);
      }
    }
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
  if(gsm_operator == "") {
    get_gsm_operator();
  }
  display.drawString(0, 0, gsm_operator);
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
    generate_upi_qr();
    // display_upi_qr("name@bank", "1.00", "Name of the receiver");
    getKeyPress();
    display.clear();
    display_menu();
  } else if(key == '4') {
    Serial.println("Settings.");
    display.clear();
    settings_menu();
  } else {
    Serial.println("Invalid option pressed on menu.");
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

int get_gsm_operator() {
  reset_array(gsm_resp);
  int at_resp_len = send_at_command("AT+COPS=3,0", gsm_resp);
  reset_array(gsm_resp);
  reset_array(gsm_operator);
  at_resp_len = send_at_command("AT+COPS?", gsm_resp);
  strcpy(gsm_operator, gsm_resp);

  // Parse the string to extract the operator
  char* ptr1 = strstr(gsm_operator, "\"");
  char mob_op[50] = {0,};
  if(ptr1 != NULL) {
    ptr1 += 1;
    char* ptr2 = strstr(ptr1, "\"");
    if(ptr2 != NULL) {
      strncpy(mob_op, ptr1, ptr2 - ptr1);
      strcpy(gsm_operator, mob_op);
    }
  }
  if(sizeof(gsm_operator) == 0) {
    strcpy(gsm_operator, "Unknown");
  }
  Serial.print("GSM operator: ");
  Serial.println(gsm_operator);
  return sizeof(gsm_operator);
}

char get_net_strength() {
  reset_array(gsm_resp);
  int at_resp_len = send_at_command("AT+CSQ", gsm_resp);
  Serial.print("Network strength: ");
  Serial.println(gsm_resp);
  reset_array(net_strength);
  strcpy(net_strength, gsm_resp);
  return sizeof(net_strength);
}

void make_call() {
  display.clear();
  display.drawString(0, 0, "MAKE CALL");
  display.drawString(0, 10, "Number:");
  display.display();
  char call_keypad_handler_ret = call_keypad_handler(call_phone_number);
  if(call_keypad_handler_ret == 'D') {
    display.clear();

    Serial.print("Dialing: ");
    Serial.println(call_phone_number);
    char dial_cmd[50] = {0,};
    strcpy(dial_cmd, "ATD");
    strcat(dial_cmd, call_phone_number);
    strcat(dial_cmd, ";");
    Serial.print("Dial command: ");
    Serial.println(dial_cmd);
    int at_resp_len = send_at_command(dial_cmd, gsm_resp);
    Serial.print("GSM response: ");
    Serial.println(gsm_resp);
    // Validate the GSM response
    int dial_complete = 0;
    if(strstr(gsm_resp, "OK") != NULL) {
      Serial.println("Ringing..");
      dial_complete = 1;
    } else {
      Serial.println("Could not make phone call..");
      dial_complete = 0;
    }

    // Update the display
    display.drawString(0, 0, "Dialing:");
    display.drawString(0, 10, call_phone_number);
    if(dial_complete) {
      display.drawString(0, 20, "Ringing..");
    } else {
      display.drawString(0, 20, "Error dialing..");
      display.display();
      delay(5000);
      display.clear();
      display_menu();
    }
    display.display();

    while(1) {
      char key = getKeyPress();
      if(key == 'A') {
        Serial.println("Call hang up.");
        reset_array(call_phone_number);
        // End the call here and check for the response. Also resend the command until OK is received.
        at_resp_len = send_at_command("ATH", gsm_resp);
        Serial.print("GSM response: ");
        Serial.println(gsm_resp);
        delay(2000);
        display.clear();
        display_menu();
      } else {
        continue;
      }
    }
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

void generate_upi_qr() {
  display.clear();
  display.drawString(0, 0, "Generate UPI QR");
  display.drawString(0, 10, "Enter amount: ");
  display.drawString(0, 20, "0.00");
  display.display();
  char amount[10];
  reset_array(amount);
  while(1) {
    char key = getKeyPress();
    if(key == 'A') {
      // Cancel
      display.clear();
      Serial.println("UPI QR amt cancelled.");
      display_menu();
    } else if(key == 'B') {
      // Backspace
      Serial.println("UPI QR amt backspace.");
      if(strlen(amount) > 0) {
        amount[strlen(amount) - 1] = '\0';
        display.clear();
        display.drawString(0, 0, "Generate UPI QR");
        display.drawString(0, 10, "Enter amount: ");
        display.drawString(0, 20, amount);
        display.display();
      }
    } else if(key == 'C') {
      // Clear
      Serial.println("UPI QR amt cleared.");
      reset_array(amount);
      display.clear();
      display.drawString(0, 0, "Generate UPI QR");
      display.drawString(0, 10, "Enter amount: ");
      display.drawString(0, 20, "00.00");
      display.display();
    } else if(key == 'D') {
      // Generate QR
      Serial.println("UPI QR amt entered, generating QR..");
      display_upi_qr("name@bank", amount, "Receiver Name");
      break;
    } else if(key == '*' || key == '#') {
      continue;
    } else {
      // Add the key to the amount
      Serial.print("Amount entered: ");
      Serial.println(amount);
      amount[strlen(amount)] = key;
      display.clear();
      display.drawString(0, 0, "Generate UPI QR");
      display.drawString(0, 10, "Enter amount: ");
      display.drawString(0, 20, amount);
      display.display();
    }
  }
  display.display();
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
    display.drawString(0, 10, "Operator: " + String(gsm_operator));
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
    Serial.println("Invalid option pressed on settings menu.");
    display.drawString(0, 50, "Invalid option!");
    display.display();
    settings_menu();
  }
}
