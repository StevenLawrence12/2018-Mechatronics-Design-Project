/*
 *Motor Control V1.0
 *
 */
//include librarys
#include<Servo.h>

//init servo objects
Servo servo_LeftMotor;
Servo servo_RightMotor;

//init pins
const int LeftMotorPin=8;
const int RightMotorPin=9;

//init variables
unsigned int leftMotorSpeed;
unsigned int rightMotorSpeed;

//drive mode; 0=stop, 1=straight, 2=left turn, 3=right turn
unsigned int driveMode=0;

//drive function
void drive(int leftSpeed,int rightSpeed)
{
  servo_LeftMotor.writeMicroseconds(leftSpeed);
  servo_RightMotor.writeMicroseconds(rightSpeed);
}

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
  switch(driveMode)
  {
    case 0:
    {
      leftMotorSpeed=1500;
      rightMotorSpeed=1500;
      Serial.println("Stopped");
    }
    case 1: 
    {
      leftMotorSpeed=1600;
      rightMotorSpeed=1600;
      Serial.println("Driving Straight");
    }
     case 2: 
    {
      leftMotorSpeed=1500;
      rightMotorSpeed=1600;
      Serial.println("Turning Left");
    }
     case 3: 
    {
      leftMotorSpeed=1500;
      rightMotorSpeed=1600;
      Serial.println("Turning Right");
    }
  }
 drive(leftMotorSpeed, rightMotorSpeed);
}
