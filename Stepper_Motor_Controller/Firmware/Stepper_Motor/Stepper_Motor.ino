#include <AccelStepper.h>
#define motorPin1 8 // IN1 on the ULN2003 driver
#define motorPin2 7 // IN2 on the ULN2003 driver
#define motorPin3 6 // IN3 on the ULN2003 driver
#define motorPin4 5 // IN4 on the ULN2003 driver

#define MotorInterfaceType 8
int stepsPerRevolution = 2048;
unsigned long pressedTime, releaseTime;
int lastState = HIGH, currentState;

AccelStepper stepper = AccelStepper(MotorInterfaceType, motorPin1, motorPin3, motorPin2, motorPin4);
int state = 0;

void runMotor()
{
  releaseTime = millis();
  if (releaseTime - pressedTime > 1000)
  {
    stepsPerRevolution = -stepsPerRevolution;
  }
  state = 1;
}
void setup() {
  pinMode(2, INPUT);
  attachInterrupt(0, runMotor, RISING);
  stepper.setMaxSpeed(1500);
  stepper.setAcceleration(1000);
}

void loop() {
  currentState = digitalRead(2);
  if (currentState == LOW && lastState == HIGH) {
    pressedTime = millis();
  }
  lastState = currentState;
  if (state == 1)
  {
    int sensorReading = analogRead(A0);
    int numberRotate = map(sensorReading, 0, 1023, 1, 10);
    stepper.setCurrentPosition(0);
    stepper.moveTo(numberRotate * stepsPerRevolution);
    stepper.runToPosition();
    state = 0;
  }
}
