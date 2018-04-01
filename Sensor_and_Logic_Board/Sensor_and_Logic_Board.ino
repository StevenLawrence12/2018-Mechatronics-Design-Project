/*
 * Sensor and Logic Board V2.0
 */
 
//Include librarys
#include<Wire.h>
#include<I2CEncoder.h>
#include<mcp_can.h>
#include<SPI.h>

//declare pins
const int sideTrigPin1=4; //forward side ultrasonic pin
const int sideEchoPin1=5; //forward side ultrasonic pin
const int sideTrigPin2=6;  //back side ultrasonic
const int sideEchoPin2=7;  //back side ultrasonic
const int frontTrigPin=8;  //front ultrasonic
const int frontEchoPin=9;  //front ultrasonic
const int SPI_CS_PIN=10; 


//init objects
I2CEncoder leftEncoder;                       
I2CEncoder rightEncoder;
MCP_CAN CAN(SPI_CS_PIN);

//init variables
double leftEncoderSpeed; //left motor encoder
double rightEncoderSpeed; //right motor encoder
int frontDistance; //distance infront of robot
int sideDistance1; //distance to the right side of robot
int sideDistance2; 
int frontStopDistance=30; //minimum distance to objects infront of robot before turning/stopping

int leftDriveMotorSpeed=0; //speed of left motor (0-255)
bool leftDriveMotorReverse=true; //sets motor in reverse or forward (false=reverse, true= forward
int rightDriveMotorSpeed=0; //speed of right motor (0-255)
bool rightDriveMotorReverse=true; //sets right motor in forward or reverse motion (false=reverse, true=forward)
//int targetSpeed;

int tempLeftDriveMotorSpeed=0;  //temporary left motor speed for a left turn
bool tempLeftDriveMotorReverse=true; //sets motor direction
int tempRightDriveMotorSpeed=0; //temporary right motor speed for a right turn
bool tempRightDriveMotorReverse=true; //sets right motor direction

//CAN id's
unsigned long motorDriveId=0x01; //ID for drive motor instructions =1

//CAN messagebufs
byte motorDriveBuf[8]; //array of 8 bytes to hold the information for drive motor instructions

//reads the ultrasonics
void ultrasonicRead(const int trigPin, const int echoPin, int *distance){
 // int oldDist=*distance;
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
/*if(abs(*distance-oldDist)>40)
*distance=oldDist;*/
}

//setup drive motor instuction array 
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

//sends a message through CAN
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
pinMode(sideTrigPin1,OUTPUT);
pinMode(sideEchoPin1,INPUT);
pinMode(sideTrigPin2,OUTPUT);
pinMode(sideEchoPin2,INPUT);

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

ultrasonicRead(sideTrigPin1,sideEchoPin1,&sideDistance1); //reads side ultrasonic pin
ultrasonicRead(sideTrigPin2,sideEchoPin2,&sideDistance2);
Serial.println(sideDistance1);
Serial.println(sideDistance2);
if((sideDistance1>=6)&&(sideDistance2>=6)&&(sideDistance1<=9)&&(sideDistance2<=9)){
  Serial.println("In range");
if((sideDistance1-sideDistance2)<0){
  Serial.println("left");
  leftDriveMotorSpeed--;
}
else if((sideDistance1-sideDistance2)>0){
  Serial.println("right");
  leftDriveMotorSpeed++;
}
}
if(sideDistance1<6){
Serial.println("Hard Left");
leftDriveMotorSpeed-=4;
}
else if(sideDistance2<6){
Serial.println("right");
leftDriveMotorSpeed++;
}
else if(sideDistance1>9){
Serial.println("Hardright");
leftDriveMotorSpeed+=4;
}
else if(sideDistance2>9){
Serial.println("left");
leftDriveMotorSpeed--;
}
else{
  if((leftEncoderSpeed-rightEncoderSpeed)>.5) {
    leftDriveMotorSpeed--;
  }
if((rightEncoderSpeed-leftEncoderSpeed)>.5)  {
    leftDriveMotorSpeed++;
  } 
}




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

Serial.println(leftDriveMotorSpeed);
Serial.println(rightDriveMotorSpeed);

/*if(leftDriveMotorSpeed>140)leftDriveMotorSpeed=130;
if(leftDriveMotorSpeed<100)leftDriveMotorSpeed=110;*/
setDriveMotorSettings(&leftDriveMotorSpeed,&leftDriveMotorReverse,&rightDriveMotorSpeed,&rightDriveMotorReverse, motorDriveBuf);

 /*for(int i=0;i<8;i++){
    Serial.print(motorDriveBuf[i]);
    Serial.print(", ");
  }Serial.println();

  Serial.print("CAN ID: ");
  Serial.println(motorDriveId);*/
  
sendCANMsg(&motorDriveId,motorDriveBuf);

}
