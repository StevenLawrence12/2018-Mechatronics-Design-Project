/*
 *Motor Control V1.0
 *
 */
//include librarys
#include<Servo.h>
#include<mcp_can.h>
#include<SPI.h>

//setup CAN 
const int SPI_CS_PIN=10;
MCP_CAN CAN(SPI_CS_PIN);

//init servo objects
Servo servo_LeftMotor;
Servo servo_RightMotor;

//init pins
const int LeftMotorPin=8;
const int RightMotorPin=9;

//init variables
unsigned int leftMotorSpeed;
unsigned int rightMotorSpeed;

//drive mode; 0=stop, 1=straight, 2=left turn, 3=right turn, 4=reverse
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

  //init CAN
  while(CAN_OK!=CAN.begin(CAN_500KBPS)
  {
    Serial.println("CAN BUS init fail");
    Serial.println("Init CAN BUS fail again");
    delay(100);
  }
  Serial.println("CAN BUS init ok!");
}

void loop() 
{
  if(Serial.available()>0)
  driveMode=Serial.read()-48;
  
  switch(driveMode)
  {
    case 0:
    {
      leftMotorSpeed=1500;
      rightMotorSpeed=1500;
      Serial.println("Stopped");
      break;
    }
    case 1: 
    {
      leftMotorSpeed=1650;
      rightMotorSpeed=1640;
      Serial.println("Driving Straight");
      break;
    }
     case 2: 
    {
      leftMotorSpeed=1450;
      rightMotorSpeed=1650;
      Serial.println("Turning Left");
      break;
    }
     case 3: 
    {
      leftMotorSpeed=1650;
      rightMotorSpeed=1450;
      Serial.println("Turning Right");
      break;
    }
    case 4:
    {
      leftMotorSpeed=1350;
      rightMotorSpeed=1360;
      Serial.println("Reverse");
      break;
    }
  }
 drive(leftMotorSpeed, rightMotorSpeed);
}
