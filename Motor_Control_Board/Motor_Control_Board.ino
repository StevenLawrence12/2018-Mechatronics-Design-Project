/*
 *Motor Control V1.0
 *
 */

#include<Servo.h>

Servo servo_LeftMotor;
Servo servo_RightMotor;

const int LeftMotorPin=8;
const int RightMotorPin=9;

void setup() 
{
  Serial.begin(9600);
  pinMode(LeftMotorPin,OUTPUT);
  servo_LeftMotor.attach(LeftMotorPin);
  pinMode(RightMotorPin,OUTPUT);
  servo_RightMotor.attach(RightMotorPin);
}

void loop() 
{
 servo_LeftMotor.writeMicroseconds(1600);
 Serial.println("drive");
 servo_RightMotor.writeMicroseconds(1600);
}
