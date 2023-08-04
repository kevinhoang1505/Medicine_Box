#include "pin_config.h"

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );
LiquidCrystal_I2C lcd(LCD_ADDRESS, 20, 4);

void setup() {
  lcd.init();
  lcd.backlight();
  pinMode(BUZZER, OUTPUT);
  Serial.begin(9600);
  Wire.begin();
  lcd.setCursor(1, 0);
  lcd.print("TU THUOC THONG MINH");
  delay(500);
  if (!SD.begin()) {
    lcd.setCursor(2, 1);
    lcd.print("KHONG CO THE NHO");
    SOUND_ON_LONG;
    ESP.restart();
  }

  uint8_t cardType = SD.cardType();
  uint8_t cardSize = SD.cardSize() / (1024 * 1024 * 1024);
  lcd.setCursor(1, 1);
  if (cardType == CARD_MMC) {
    lcd.printf("MMC %d GB", cardSize);
  } else if (cardType == CARD_SD) {
    lcd.printf("SDSC %d GB", cardSize);
  } else if (cardType == CARD_SDHC) {
    lcd.printf("SDHC %d GB", cardSize);
  } else {
    lcd.printf("UNKNOWN %d GB", cardSize);
  }

  File file = SD.open("/dataset.csv");
  while (file.available())
  {
    character = file.read();
    if (character == ',')
    {
      j++;
      continue;
    } else if (character == '\n')
    {
      i++;
      j = 0;
      continue;
    } else
      dataset[i][j] += character;
  }
  file.close();
  delay(1000);
  lcd.clear();
}

void loop() {
  char key = keypad.getKey();
  if (key) {
    SOUND_ON;
    switch (key) {
      case '*':
        lcd.clear();
        id_string.remove(id_string.length() - 1);
        break;
      case '#':
        slot_id = 0;
        for (int i = 0; i < 9; i++)
        {
          if (dataset[i][0] == id_string)
          {
            SOUND_ON;
            slot_id = (int) dataset[i][1].toInt();
            Serial.println(slot_id, HEX);
          } else
          {
            if (slot_id == 0 && slot_id == 9)
            {
              slot_id = 0;
            }
          }
        }
        if (slot_id > 0)
        {
          Wire.beginTransmission(slot_id); // Bắt đầu truyền dữ liệu về địa chỉ số 6
          Wire.write(0b0100); // Truyền ký tự 0b0100
          byte bus_status = Wire.endTransmission(); // kết thúc truyền dữ liệu
          if (bus_status == 0)
          {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.printf("LAY THUOC O SO %d", slot_id);
            while (1)
            {
              if (Wire.available())
              {
                if (Wire.read() == 0b11111111)
                {
                  break;
                }
              }
              Wire.requestFrom(slot_id, 1);
              delay(5);
            }
            SOUND_ON;
            lcd.clear();
          }
          delay(500);
        } else
        {
          lcd.clear();
          lcd.print("SAI ID");
          SOUND_ON_LONG;
          lcd.clear();
        }
        break;
      default:
        if (id_string.length() < 10)
        {
          id_string += key;
        }
        break;
    }
  } else
  {
    lcd.setCursor(0, 0);
    lcd.printf("NHAP ID: %s", id_string);
  }
}
