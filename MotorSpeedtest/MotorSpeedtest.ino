#include<Servo.h>

int leftMotorPin=8;
int rightMotorPin=9;

Servo leftMotor;
Servo rightMotor;

int leftMotorSpeed=1615;
int rightMotorSpeed=1600;

void setup() {
pinMode(leftMotorPin,OUTPUT);
pinMode(rightMotorPin,OUTPUT);

leftMotor.attach(leftMotorPin);
rightMotor.attach(rightMotorPin);
}

void loop() {
leftMotor.writeMicroseconds(leftMotorSpeed);
rightMotor.writeMicroseconds(rightMotorSpeed);
}
