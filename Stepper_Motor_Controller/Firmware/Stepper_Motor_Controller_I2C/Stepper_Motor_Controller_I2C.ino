//Includes the Arduino Library
#include <Wire.h>
#include <Stepper.h>
// Defines the motor
#define STEPPER_PIN_1 8 // IN1 on the ULN2003 driver
#define STEPPER_PIN_2 7 // IN2 on the ULN2003 driver
#define STEPPER_PIN_3 6 // IN3 on the ULN2003 driver
#define STEPPER_PIN_4 5 // IN4 on the ULN2003 driver
#define SPEED 30    // Motor Speed
// Defines Adress
#define ADDRESS 0x01 // Motor I2C Address

int state = -1;
const int stepsPerRevolution = 1024;
byte receive_data, round_number;

Stepper myStepper = Stepper(stepsPerRevolution, STEPPER_PIN_1, STEPPER_PIN_3, STEPPER_PIN_2, STEPPER_PIN_4);

void halt()
{
  digitalWrite(STEPPER_PIN_1, LOW);
  digitalWrite(STEPPER_PIN_2, LOW);
  digitalWrite(STEPPER_PIN_3, LOW);
  digitalWrite(STEPPER_PIN_4, LOW);
}
void runStep(int rpm)
{
  if (receive_data < 0b10000)
  {
    receive_data = Wire.read(); // 0b1110;
    round_number = receive_data >> 1;
    state = receive_data & 0b0001;
  }
}
void txHandler(void)
{
  if (state == -1)
  {
    Wire.write(0b11111111);
  }
}
void startMotor()
{
  state = 1;
  round_number = 1;
}

void stopMotor()
{
  state = -1;
  round_number = 0;
}
void setup() {
  attachInterrupt(0, startMotor, FALLING);
  attachInterrupt(1, stopMotor, FALLING);
  myStepper.setSpeed(SPEED);
  Wire.begin(ADDRESS);
  Wire.onReceive(runStep);
  Wire.onRequest(txHandler);
}

void loop() {
  if (state == 1)
  {
    myStepper.step(round_number * stepsPerRevolution);
  }
  else if (state == 0)
  {
    myStepper.step(-(round_number * stepsPerRevolution));
  }
  state = -1;
  halt();
}
