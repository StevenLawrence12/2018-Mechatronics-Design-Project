/*
 * Sensor and Logic Board V2.0
 */
//Include librarys
#include<Wire.h>
#include<I2CEncoder.h>
#include<mcp_can.h>
#include<SPI.h>

//define pin numbers
const int trigPin=8;  //front ultrasonic
const int echoPin=9;  //front ultrasonic
const int SPI_CS_PIN=10; 

I2CEncoder leftEncoder;
I2CEncoder rightEncoder;
MCP_CAN CAN(SPI_CS_PIN);

//init variables
double leftEncoderSpeed;
double rightEncoderSpeed;
int frontDistance;
int frontStopDistance=10;

//CAN id's
int motorDrive=0x3;

//CAN messagebufs
byte motorDriveBuf[8];

//checks the distance infront of the robot
void frontDistanceRead()
{
  //clears the trigpin
digitalWrite(trigPin,LOW);
delayMicroseconds(2);

//set trigpin high for 10 microseconds to send pulse out
digitalWrite(trigPin,HIGH);
delayMicroseconds(10);
digitalWrite(trigPin,LOW);

//read the echopin
long duration=pulseIn(echoPin,HIGH);

//calculate the distance 0.034cm/us
frontDistance=duration*0.034/2;
}


void setup() 
{ 
Wire.begin();
Serial.begin(9600);

//init encoders 

//leftEncoder.init((/*put ft/motor output shaft rotations*/)*MOTOR_393_SPEED_ROTATIONS,MOTOR_393_TIME_DELTA);
//leftEncoder.setReversed(/*true or false*/);
//rightEncoder.init((/*put ft/motor output shaft rotations*/)*MOTOR_393_SPEED_ROTATIONS,MOTOR_393_TIME_DELTA);
//rightEncoder.setReversed(/*true or false*/);


//init CAN
while(CAN_OK!=CAN.begin(CAN_500KBPS))
{
  Serial.println("CAN BUS init fail");
  Serial.println("Init CAN BUS fail again");
  delay(100);
}
Serial.println("CAN BUS init ok!");

//pinmodes
pinMode(trigPin,OUTPUT);
pinMode(echoPin,INPUT);

}

void loop() 
{

  //read left and right encoders
leftEncoderSpeed=leftEncoder.getSpeed();
rightEncoderSpeed=rightEncoder.getSpeed();
Serial.print("Left encoder speed: ");
Serial.println(leftEncoderSpeed);
Serial.print("Right encoder speed: ");
Serial.println(rightEncoderSpeed);

frontDistanceRead();
Serial.print("Front distance");
Serial.println(frontDistance);
/*if(frontDistance<=frontStopDistance)
{
  motorDriveBuf[0]=
  sendMsgBuf(motorDrive,0,8,motorDriveBuf);
}*/

}
