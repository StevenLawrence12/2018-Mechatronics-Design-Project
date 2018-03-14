/*
 * Sensor and Logic Board V2.0
 */
//Include librarys
#include<Wire.h>
#include<I2CEncoder.h>
#include<mcp_can.h>
#include<SPI.h>

//define pin numbers
const int sideTrigPin=4;  //side ultrasonic
const int sideEchoPin=7;  //side ultrasonic

const int frontTrigPin=8;  //front ultrasonic
const int frontEchoPin=9;  //front ultrasonic
const int SPI_CS_PIN=10; 


//init objects
I2CEncoder leftEncoder;
I2CEncoder rightEncoder;
MCP_CAN CAN(SPI_CS_PIN);

//init variables
double leftEncoderSpeed; 
double rightEncoderSpeed;
int frontDistance;
//int sideDistance;
int frontStopDistance=20;
//int driveMode=0; //0=wall follow 1=free drive

int leftDriveMotorSpeed=150;
//bool leftDriveMotorReverse;
int rightDriveMotorSpeed=150;
//bool rightDriveMotorReverse;
//int targetSpeed;

//CAN id's
int motorDriveId=0x01;

//CAN messagebufs
byte motorDriveBuf[8];

//reads the ultrasonics
void ultrasonicRead(const int trigPin, const int echoPin, int *distance)
{
  //clears the frontTrigPin
digitalWrite(trigPin,LOW);
delayMicroseconds(2);

//set frontTrigPin high for 10 microseconds to send pulse out
digitalWrite(trigPin,HIGH);
delayMicroseconds(10);
digitalWrite(trigPin,LOW);

//read the frontEchoPin
long duration=pulseIn(echoPin,HIGH);

//calculate the distance 0.034cm/us
*distance=duration*0.034/2;
}


void setup() 
{ 
Wire.begin();
Serial.begin(9600);

//init encoders 

leftEncoder.init((10.3)*MOTOR_393_SPEED_ROTATIONS,MOTOR_393_TIME_DELTA);
leftEncoder.setReversed(false);
rightEncoder.init((10.3)*MOTOR_393_SPEED_ROTATIONS,MOTOR_393_TIME_DELTA);
rightEncoder.setReversed(true);


//init CAN
while(CAN_OK!=CAN.begin(CAN_500KBPS))
{
  Serial.println("CAN BUS init fail");
  Serial.println("Init CAN BUS fail again");
  delay(100);
}
Serial.println("CAN BUS init ok!");

//pinmodes
pinMode(frontTrigPin,OUTPUT);
pinMode(frontEchoPin,INPUT);
/*pinMode(sideTrigPin,OUTPUT);
pinMode(sideTrigPin,INPUT);*/

motorDriveBuf[1]=0x01;
motorDriveBuf[3]=0x01;

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

  if((leftEncoderSpeed-rightEncoderSpeed)>.5) 
  {
    leftDriveMotorSpeed--;
  }
  if((rightEncoderSpeed-leftEncoderSpeed)>.5)  
  {
    leftDriveMotorSpeed++;
  }

motorDriveBuf[0]=rightDriveMotorSpeed;
motorDriveBuf[2]=leftDriveMotorSpeed;

//send message
byte CANTx=CAN.sendMsgBuf(motorDriveId,0,8,motorDriveBuf);
if(CANTx==CAN_OK)
 Serial.println("Message sent successfully");
 else
 {
 Serial.print("Message not sent. Error code: ");
 Serial.println(CANTx);
 }

 
ultrasonicRead(frontTrigPin,frontEchoPin,&frontDistance);  //reads front ultrasonic pin
//if(driveMode=1)
/*if(frontDistance<=frontStopDistance)
{
 motorDriveBuf[0]=0x0;
 motorDriveBuf[1]=0x2;
 motorDriveBuf[2]=0x0;
 motorDriveBuf[3]=0x2;
 byte CANTx=CAN.sendMsgBuf(motorDriveId,0,8,motorDriveBuf);
 if(CANTx==CAN_OK)
 Serial.println("Message sent successfully");
 else
 {
 Serial.print("Message not sent. Error code: ");
 Serial.println(CANTx); 
 }
}

else
{
 motorDriveBuf[0]=0x96;
 motorDriveBuf[1]=0x1;
 motorDriveBuf[2]=0x8c;
 motorDriveBuf[3]=0x1;
 byte CANTx=CAN.sendMsgBuf(motorDriveId,0,8,motorDriveBuf);
 if(CANTx==CAN_OK)
 Serial.println("Message sent successfully");
 else
 {
 Serial.print("Message not sent. Error code: ");
 Serial.println(CANTx); 
 }
}*/


/*ultrasonicRead(sideTrigPin,sideEchoPin,&sideDistance); //reads side ultrasonic pin
if(sideDistance<7)
{
  motorDriveBuf[2]--;
}
else if( sideDistance>10)
{
  motorDriveBuf[0]--;
}*/

}

