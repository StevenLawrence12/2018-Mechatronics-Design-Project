/*
 * Sensor and Logic Board V2.0
 */
 
//Include librarys
#include<Wire.h>
#include<I2CEncoder.h>
#include<mcp_can.h>
#include<SPI.h>

//declare pins
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
int sideDistance;
int frontStopDistance=10;

int leftDriveMotorSpeed=0;
bool leftDriveMotorReverse=true;
int rightDriveMotorSpeed=0;
bool rightDriveMotorReverse=true;
//int targetSpeed;

int tempLeftDriveMotorSpeed=0;
bool tempLeftDriveMotorReverse=true;
int tempRightDriveMotorSpeed=0;
bool tempRightDriveMotorReverse=true;

//CAN id's
unsigned long motorDriveId=0x01;

//CAN messagebufs
byte motorDriveBuf[8];

//reads the ultrasonics
void ultrasonicRead(const int trigPin, const int echoPin, int *distance){
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

void setDriveMotorSettings(int *leftSpeed,bool *leftRev,int *rightSpeed,bool *rightRev, byte *buf){
  /*Serial.println(*leftSpeed);
  Serial.println(*rightSpeed);*/
  buf[0]=*leftSpeed;  
  buf[1]=*leftRev;
  buf[2]=*rightSpeed;
  buf[3]=*rightRev;
  buf[4]=0;
  buf[5]=0;
  buf[6]=0;
  buf[7]=0;
}

void sendCANMsg(unsigned long *msgId,byte *msgBuf){
 byte CANTx=CAN.sendMsgBuf(*msgId,0,8,msgBuf);
 if(CANTx==CAN_OK)
 Serial.println("Message sent successfully");
 else
 {
 Serial.print("Message not sent. Error code: ");
 Serial.println(CANTx); 
 }
}


void setup() 
{ 
Wire.begin();
Serial.begin(9600);

//init encoders 
leftEncoder.init((32.4)*MOTOR_393_SPEED_ROTATIONS,MOTOR_393_TIME_DELTA);
leftEncoder.setReversed(false);
rightEncoder.init((32.4)*MOTOR_393_SPEED_ROTATIONS,MOTOR_393_TIME_DELTA);
rightEncoder.setReversed(true);


//init CAN
while(CAN_OK!=CAN.begin(CAN_500KBPS)){
  Serial.println("CAN BUS init fail");
  Serial.println("Init CAN BUS fail again");
  delay(100);
}Serial.println("CAN BUS init ok!");

//pinmodes
pinMode(frontTrigPin,OUTPUT);
pinMode(frontEchoPin,INPUT);
pinMode(sideTrigPin,OUTPUT);
pinMode(sideTrigPin,INPUT);

leftDriveMotorSpeed=120;
rightDriveMotorSpeed=120;
}

void loop() 
{

//read encoders
leftEncoderSpeed=leftEncoder.getSpeed();
rightEncoderSpeed=rightEncoder.getSpeed();

if((leftEncoderSpeed-rightEncoderSpeed)>.5) {
    leftDriveMotorSpeed--;
  }
if((rightEncoderSpeed-leftEncoderSpeed)>.5)  {
    leftDriveMotorSpeed++;
  } 

/*Serial.print("Left encoder speed: ");
Serial.print(leftEncoderSpeed/60);
Serial.println("cm/s");
Serial.print("Right encoder speed: ");
Serial.print(rightEncoderSpeed/60);
Serial.println("cm/s");*/

ultrasonicRead(sideTrigPin,sideEchoPin,&sideDistance); //reads side ultrasonic pin

/*if(sideDistance<7)
{
  leftDriveMotorSpeed--;
}
else if( sideDistance>10)
{
  leftDriveMotorSpeed++;
}*/

ultrasonicRead(frontTrigPin,frontEchoPin,&frontDistance);  //reads front ultrasonic pin

Serial.print("Front distance: ");
Serial.println(frontDistance);
//if a wall is too close, make a left turn;
while(frontDistance<=frontStopDistance)
{
  Serial.println("Turning");
 tempLeftDriveMotorSpeed=100 ;
 tempLeftDriveMotorReverse=0;
 tempRightDriveMotorSpeed=150;
 tempRightDriveMotorReverse=1;
 setDriveMotorSettings(&tempLeftDriveMotorSpeed,&tempLeftDriveMotorReverse,&tempRightDriveMotorSpeed,&tempRightDriveMotorReverse, motorDriveBuf);
 sendCANMsg(&motorDriveId,motorDriveBuf);
 ultrasonicRead(frontTrigPin,frontEchoPin,&frontDistance);  //reads front ultrasonic pin

}

/*Serial.println(leftDriveMotorSpeed);
Serial.println(rightDriveMotorSpeed);*/
setDriveMotorSettings(&leftDriveMotorSpeed,&leftDriveMotorReverse,&rightDriveMotorSpeed,&rightDriveMotorReverse, motorDriveBuf);

 /*for(int i=0;i<8;i++){
    Serial.print(motorDriveBuf[i]);
    Serial.print(", ");
  }Serial.println();

  Serial.print("CAN ID: ");
  Serial.println(motorDriveId);*/
  
sendCANMsg(&motorDriveId,motorDriveBuf);

}
