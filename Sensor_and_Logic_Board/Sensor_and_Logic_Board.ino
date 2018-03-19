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

void setDriveMotorSettings(int *leftSpeed,bool *leftRev,int *rightSpeed,bool *rightRev){
  motorDriveBuf[0]=leftSpeed;
  motorDriveBuf[1]=leftRev;
  motorDriveBuf[2]=rightSpeed;
  motorDriveBuf[3]=rightRev;
  motorDriveBuf[4]=0;
  motorDriveBuf[5]=0;
  motorDriveBuf[6]=0;
  motorDriveBuf[7]=0;
}

void sendCANMsg(unsigned long *msgId,byte *msgBuf){
 byte CANTx=CAN.sendMsgBuf(msgId,0,8,msgBuf);
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

leftDriveMotorSpeed=150;
rightDriveMotorSpeed=150;
}

void loop() 
{


ultrasonicRead(frontTrigPin,frontEchoPin,&frontDistance);  //reads front ultrasonic pin

//read encoders
leftEncoderSpeed=leftEncoder.getSpeed();
rightEncoderSpeed=rightEncoder.getSpeed();

if((leftEncoderSpeed-rightEncoderSpeed)>.5) {
    leftDriveMotorSpeed--;
  }
  if((rightEncoderSpeed-leftEncoderSpeed)>.5)  {
    leftDriveMotorSpeed++;
  } 

Serial.print("Left encoder speed: ");
Serial.print(leftEncoderSpeed/60);
Serial.println("cm/s");
Serial.print("Right encoder speed: ");
Serial.print(rightEncoderSpeed/60);
Serial.println("cm/s");

ultrasonicRead(sideTrigPin,sideEchoPin,&sideDistance); //reads side ultrasonic pin

if(sideDistance<7)
{
  leftDriveMotorSpeed--;
}
else if( sideDistance>10)
{
  leftDriveMotorSpeed++;
}

//if a wall is too close, make a left turn;
if(frontDistance<=frontStopDistance)
{
 leftDriveMotorSpeed=50;
 leftDriveMotorReverse=0;
 rightDriveMotorSpeed=150;
 rightDriveMotorReverse=1;
}

setDriveMotorSettings(&leftDriveMotorSpeed,&leftDriveMotorReverse,&rightDriveMotorSpeed,&rightDriveMotorReverse);
sendCANMsg(&motorDriveId,motorDriveBuf);

}
