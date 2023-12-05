#include "pin_config.h"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic1 = NULL;
BLECharacteristic* pCharacteristic2 = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool export_state = 0;
unsigned long previousMillis = 0;
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID                        "9cf8e0c0-0cca-4303-b6e9-06608ed4ce24"
#define NUMBER_MEDICINE_CHARACTERISTIC_UUID "9cf8e0c1-0cca-4303-b6e9-06608ed4ce24"
#define SLOT_ID_CHARACTERISTIC_UUID         "9cf8e0c2-0cca-4303-b6e9-06608ed4ce24"

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );
LiquidCrystal_I2C lcd(LCD_ADDRESS, 20, 4);

void queryData(String string_id)
{

}
void getData(int *list_distance)
{
  byte c[2];
  int ans;
  float distance = 0;
  for (int i = 0; i < 9; i++)
  {
    Wire.beginTransmission(sensor_address[i]);
    Wire.write(0x5E);
    ans = Wire.endTransmission();
    if (ans == 0)
    {
      Wire.requestFrom(sensor_address[i], 2);
      c[0] = Wire.read();
      c[1] = Wire.read();
      distance = (c[0] * 16 + c[1]) / 16.0;
      if (distance > 50)
      {
        distance = 0;
        //Serial.printf("Distance[%d] = %d\n", i, 0);
        list_distance[i] = 0;
      } else
      {
        distance -= 10;
        //Serial.printf("Distance[%d] = %d\n", i, int(4 - (distance / 10)));
        list_distance[i] = int(4 - (distance / 10));
      }
    }
  }
}

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic)
    {
      std::string received_data = pCharacteristic->getValue();
      if (received_data.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        receive_string_data = "";
        for (int i = 0; i < received_data.length(); i++)
          receive_string_data += received_data[i];
        Serial.print(receive_string_data);
        Serial.println();
        Serial.println("*********");
        id_string = String(receive_string_data);
        export_state = 1;
        delay(10);
      }
    }
};

void setup() {
  lcd.init();
  lcd.backlight();
  pinMode(BUZZER, OUTPUT);
  Serial.begin(9600);
  Wire.begin();
  lcd.setCursor(1, 0);
  lcd.print("TU THUOC THONG MINH");
  Serial.println("Starting BLE work!");
  pinMode(0, INPUT_PULLUP);
  delay(2000);
  lcd.clear();
  BLEDevice::setMTU(512);
  BLEDevice::init("MedicineBox");
  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic1 = pService->createCharacteristic(
                       NUMBER_MEDICINE_CHARACTERISTIC_UUID,
                       BLECharacteristic::PROPERTY_NOTIFY
                     );
  //  pCharacteristic1->addDescriptor("NUMBER MEDICINE");
  //  pCharacteristic1->setValue("Hello World 1");

  pCharacteristic2 = pService->createCharacteristic(
                       SLOT_ID_CHARACTERISTIC_UUID,
                       BLECharacteristic::PROPERTY_WRITE
                     );
  //  pCharacteristic2->addDescriptor("CHECK SLOT ID");
  //  pCharacteristic2->setValue("Hello World 2");
  pCharacteristic2->setCallbacks(new MyCallbacks());
  pService->start();
  pServer->getAdvertising()->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  //  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  //  pAdvertising->addServiceUUID(SERVICE_UUID);
  //  pAdvertising->setScanResponse(true);
  //  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  //  pAdvertising->setMinPreferred(0x12);
  Serial.println("Characteristic defined! Now you can read it in your phone!");
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
        export_state = 1;
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
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }
  if (export_state == 1)
  {
    slot_id = 0;
    for (int i = 0; i < 9; i++)
    {
      if (dataset[i] == id_string)
      {
        SOUND_ON;
        slot_id = i + 1;
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
      Wire.write(0b0110); // Truyền ký tự 0b0100
      byte bus_status = Wire.endTransmission(); // kết thúc truyền dữ liệu
      if (bus_status == 0)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.printf("LAY THUOC O SO %d", slot_id);
        if (deviceConnected) {
          String export_slot = "export_slot:";
          export_slot += slot_id;
          pCharacteristic1->setValue(export_slot.c_str());
          pCharacteristic1->notify();
        }
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
    } else
    {
      lcd.clear();
      lcd.print("SAI ID");
      SOUND_ON_LONG;
      if (deviceConnected) {
        pCharacteristic1->setValue("msg:Sai mã thuốc");
        pCharacteristic1->notify();
        delay(10);
      }
      lcd.clear();
    }
    if (deviceConnected) {
      getData(number_medicine);
      String number_medicine_str = "[";
      for (int i = 0; i < 8; i++)
      {
        number_medicine_str += number_medicine[i];
        number_medicine_str += ",";
      }
      number_medicine_str += number_medicine[8];
      number_medicine_str += "]";
      Serial.println(number_medicine_str);
      pCharacteristic1->setValue(number_medicine_str.c_str());
      pCharacteristic1->notify();
      previousMillis = millis();
      delay(10); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    }
    export_state = 0;
    slot_id = 0;
  }
}
