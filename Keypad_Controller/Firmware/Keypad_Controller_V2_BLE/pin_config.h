#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define BUZZER 17
#define LCD_ADDRESS 0x3F
#define SOUND_ON digitalWrite(BUZZER, HIGH); delay(100); digitalWrite(BUZZER, LOW);
#define SOUND_ON_LONG digitalWrite(BUZZER, HIGH); delay(1000); digitalWrite(BUZZER, LOW);
#define ROW_NUM     4 // hàng
#define COLUMN_NUM  3 // cột

const String dataset[9] = {"1231","1232","1233","1234","1235","1236","1237","1238","1239"};
byte pin_rows[ROW_NUM] = {34, 35, 32, 33}; // GPIO18, GPIO5, GPIO17, GPIO16 chân các hàng
byte pin_column[COLUMN_NUM] = {27, 26, 25};  // GPIO4, GPIO0, GPIO2 chân các cột
const char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
const int sensor_address[9] = {0x30, 0x38, 0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70};

String id_string = "", receive_string_data = "";
int number_medicine[9] = {0};
int slot_id = -1;
char character;
unsigned int i = 0, j = 0;
