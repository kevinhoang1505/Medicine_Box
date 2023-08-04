#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define BUZZER 17
#define LCD_ADDRESS 0x3F
#define SOUND_ON digitalWrite(BUZZER, HIGH); delay(100); digitalWrite(BUZZER, LOW);
#define SOUND_ON_LONG digitalWrite(BUZZER, HIGH); delay(1000); digitalWrite(BUZZER, LOW);
#define ROW_NUM     4 // hàng
#define COLUMN_NUM  3 // cột

char character;
unsigned int i = 0, j = 0;
String dataset[9][2] = {};
byte pin_rows[ROW_NUM] = {34, 35, 32, 33}; // GPIO18, GPIO5, GPIO17, GPIO16 chân các hàng
byte pin_column[COLUMN_NUM] = {27, 26, 25};  // GPIO4, GPIO0, GPIO2 chân các cột
String id_string = "";
int slot_id = -1;
char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
