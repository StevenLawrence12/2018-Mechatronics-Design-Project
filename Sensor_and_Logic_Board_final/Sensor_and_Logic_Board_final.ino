//include librarys
#include<mcp_can.h>
#include<SPI.h>
#include<I2CEncoder.h>
#include<Wire.h>

//declare pins
const int sideUlt1Pin=7;
const int sideUlt2Pin=8;
const int frontUltrasonicPin=9;
const int SPI_CS_PIN=10;
const int hallPin=A3;

//create objects
MCP_CAN CAN(SPI_CS_PIN);
I2CEncoder leftEncoder;
I2CEncoder rightEncoder;

//init variables
unsigned int stage=0;
//other motor/servo variables
byte huggingArmPos=0;
byte extendArmPos=0;
byte swingArmSpeed=0;
//hall effect sensor variables
int hallEffectRead;
int hallEffectMin=500;
int hallEffectMax=520;
//distance variables
unsigned int frontStopDist=20;
//Drive Motor variables
byte leftMotorSpeed=0;
byte rightMotorSpeed=0;
byte leftMotorRev=0;
byte rightMotorRev=0;
//ultrasonic variables
long frontDistance;
//Encoder variables
long leftEncodRawPos;
long rightEncodRawPos;
double leftEncodSpeed;
double rightEncodSpeed;
//connect motor variables

//constants
int HUGARMFINPOS=100;

//init CAN variables
unsigned long driveMotorsId=0x01;
byte driveMotorsBuf[8]={0,0,0,0,0,0,0,0}; //0=leftMotorSpeed, 1=rightMotorSpeed, 2=leftMotorRev, 3=rightMotorRev
unsigned long miscMotorsId=0x02;
byte miscMotorsBuf[8]={0,0,0,0,0,0,0,0}; //0=hug arm position, 1=extending arm position, 2=swinging arm speed;

//function prototypes
void send_CAN_Msg(unsigned long *msgId,byte *msgBuf);
void set_CAN_TX_Buf(byte *buf,byte *b0=0, byte *b1=0, byte *b2=0, byte *b3=0, byte *b4=0, byte *b5=0, byte *b6=0, byte *b7=0);
long ultrasonic_Ping(const int ultPin);
void turn_90_Deg_Left();

void setup() {
Serial.begin(9600);
Wire.begin();

//init encoders
leftEncoder.init((31.9186)*MOTOR_393_SPEED_ROTATIONS,MOTOR_393_TIME_DELTA);
leftEncoder.setReversed(false);
rightEncoder.init((31.9186)*MOTOR_393_SPEED_ROTATIONS,MOTOR_393_TIME_DELTA);
rightEncoder.setReversed(true);

while(CAN_OK!=CAN.begin(CAN_1000KBPS)){
  Serial.println("CAN BUS init fail");
  Serial.println("Init CAN BUS fail again");
  delay(100);
}Serial.println("CAN BUS init ok!");

set_CAN_TX_Buf(miscMotorsBuf,&huggingArmPos,&extendArmPos,&swingArmSpeed);
send_CAN_Msg(&miscMotorsId,miscMotorsBuf);

}

void loop() {
switch(stage){
case 0:{ //inital start, drive straight to wall, make left turn
Serial.println("Stage 0");
//initalize case
/*********************************/
//Turn on drive motors

//start driving
leftMotorSpeed=115;
rightMotorSpeed=100;
leftMotorRev=0;
rightMotorRev=0;
set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
Serial.println("Forward");
send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
/**********************************/
while(stage==0){
  //Code to drive straight
  /***********************************/
  //Drive control with correction
  leftEncodSpeed=leftEncoder.getSpeed();
  rightEncodSpeed=rightEncoder.getSpeed();

  Serial.print("LeftSpeed: ");
  Serial.println(leftEncodSpeed);
  Serial.print("RightSpeed: ");
  Serial.println(rightEncodSpeed);
  if(leftEncodSpeed>rightEncodSpeed)
  leftMotorSpeed--;
  else if(rightEncodSpeed>leftEncodSpeed)
  leftMotorSpeed++;
  set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
  Serial.println("Forward correction");
  send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
  /***********************************/
  
  //Check front ultrasonic
  frontDistance=ultrasonic_Ping(frontUltrasonicPin);
  /*Serial.print("Front distance: ");
  Serial.println(frontDistance);*/
  //Code to make a 90 degree left turn
  /************************************/
  if(frontDistance<=frontStopDist){
    turn_90_Deg_Left();
    stage++;
  }
/************************************/
}
break;
}
case 1:{ //Wall follow code with the obtaining of the tesseract to move on
Serial.println("Stage 1");
/******************************/
//Turn on hall effect sensor
leftMotorSpeed=0;
rightMotorSpeed=0;
set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
pinMode(hallPin,INPUT);
//Turn on swinging arm
/*swingArmSpeed=255;
set_CAN_TX_Buf(miscMotorsBuf,&huggingArmPos,&extendArmPos,&swingArmSpeed);
send_CAN_Msg(&miscMotorsId,miscMotorsBuf);*/
//Turn on side ultrasonics
//declare hall effect sensor read counter
unsigned int hallConsecRead=0;
/*****************************/

while(stage==1){
//Init stage 1

//Code to follow wall
/******************************************/
//check side ultrasonics
//compare side distances
//make proper driving correction
/*****************************************/

//Check front ultrasonic
frontDistance=ultrasonic_Ping(frontUltrasonicPin);

//Code to make a 90 dgree left turn after seeing wall
/************************************************/
//If front distance is less than 20cm

/*if(frontDistance<=frontStopDist){
  leftMotorSpeed=0;
    rightMotorSpeed=0;
    set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
    send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
    turn_90_Deg_Left();
}*/
/**********************************************/


//Code to see if we have a tesseract
/***********************************/
//read hall effect sensor
hallEffectRead=analogRead(hallPin);
//If halleffect sensor sees the tesseract
Serial.print("hall effect: ");
Serial.println(hallEffectRead);
if((hallEffectRead<hallEffectMin)||(hallEffectRead>hallEffectMax))
{
  hallConsecRead++;
  if(hallConsecRead>=4){
  //increase stage
  stage++;
  Serial.println("Tesseract obtained! ");
  }
}
else 
hallConsecRead=0;
/**************************************/
}
break;
}

case 2:{  //Find pyramid 
  Serial.println("Stage 2");
  //Stage initialization code
  /*************************************/
//Turn off side ultrasonics
//Turn on IR receivers
  /*************************************/

  //Robot movement path code
  /*****************************/


  /****************************/

  //Code after pyramid is seen
  /*********************************/
//If IR receiver sees pyramid signal
//Stop
//Drive straight

//Increase stage
stage++;
  /********************************/

  break;
}

case 3:{ //Deposit tesseract code


  Serial.println("Stage 3");
  //Init stage code
  /*********************/
  //Turn off IR sensors
  //Turn off front ultrasonic
  //Turn on hugging arm
  //Turn on tipping arm
  /***********************/

  //Tipping code
  /*********************/
//rotate hugging arm
huggingArmPos=HUGARMFINPOS;
set_CAN_TX_Buf(miscMotorsBuf,&huggingArmPos,&extendArmPos,&swingArmSpeed);
send_CAN_Msg(&miscMotorsId,miscMotorsBuf);

//Delay 2 seconds
delay(2000);

//Extend tipping arm
extendArmPos=160;
set_CAN_TX_Buf(miscMotorsBuf,&huggingArmPos,&extendArmPos,&swingArmSpeed);
send_CAN_Msg(&miscMotorsId,miscMotorsBuf);

//Delay 2 seconds
delay(2000);

//Drive backwards ceratain distance

/**********************************************/
leftEncoder.zero();
rightEncoder.zero();

leftEncodRawPos=leftEncoder.getRawPosition();
rightEncodRawPos=rightEncoder.getRawPosition();
while((leftEncodRawPos>-700)||(rightEncodRawPos>-700)){
  if(leftEncodRawPos<=-700){
    leftMotorSpeed=0;
    rightMotorSpeed=120;
    leftMotorRev=1;
    rightMotorRev=1;
    set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
    send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
  }
  else if(rightEncodRawPos<=-700){
    leftMotorSpeed=120;
    rightMotorSpeed=0;
    leftMotorRev=1;
    rightMotorRev=1;
    set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
    send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
  }
  else{
   leftMotorSpeed=100;
   rightMotorSpeed=100;
   leftMotorRev=1;
   rightMotorRev=1;
   set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
   send_CAN_Msg(&driveMotorsId,driveMotorsBuf); 
  }
  leftEncodRawPos=leftEncoder.getRawPosition();
rightEncodRawPos=rightEncoder.getRawPosition();
  Serial.print("Left encoder raw pos: ");
  Serial.println(leftEncodRawPos);
  Serial.print("Right encoder raw pos: ");
  Serial.println(rightEncodRawPos);
}
//Stop
   leftMotorSpeed=0;
   rightMotorSpeed=0;
   leftMotorRev=0;
   rightMotorRev=0;
   set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
   send_CAN_Msg(&driveMotorsId,driveMotorsBuf); 
/*************************************************/


//retract tipping arm
extendArmPos=0;
set_CAN_TX_Buf(miscMotorsBuf,&huggingArmPos,&extendArmPos,&swingArmSpeed);
send_CAN_Msg(&miscMotorsId,miscMotorsBuf);

//Delay 1 second
delay(1000);

//retract hugging arm
huggingArmPos=0;
set_CAN_TX_Buf(miscMotorsBuf,&huggingArmPos,&extendArmPos,&swingArmSpeed);
send_CAN_Msg(&miscMotorsId,miscMotorsBuf);

//
//Increase stage
  /**********************/
  stage++;
  /*********************/

  break;
}

case 4:{//Code to turn everything off (end)
  Serial.println("Stage 4");
  /**************************/
//Turn off all sensors
  /**************************/
}
  
}

}

//Function definitions
void send_CAN_Msg(unsigned long *msgId,byte *msgBuf){
 byte CANTx=CAN.sendMsgBuf(*msgId,0,8,msgBuf);
 if(CANTx==CAN_OK)
 Serial.println("Message sent successfully");
 else
 {
 Serial.print("Message not sent. Error code: ");
 Serial.println(CANTx); 
 }
}

void set_CAN_TX_Buf(byte *buf,byte *b0=0, byte *b1=0, byte *b2=0, byte *b3=0, byte *b4=0, byte *b5=0, byte *b6=0, byte *b7=0){
  buf[0]=*b0;
  buf[1]=*b1;
  buf[2]=*b2;
  buf[3]=*b3;
  buf[4]=*b4;
  buf[5]=*b5;
  buf[6]=*b6;
  buf[7]=*b7;
}

long ultrasonic_Ping(const int ultPin){
  long duration, cm;

  pinMode(ultPin, OUTPUT);
  digitalWrite(ultPin, LOW);
  delayMicroseconds(2);
  digitalWrite(ultPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(ultPin, LOW);
  
  pinMode(ultPin, INPUT);
  duration = pulseIn(ultPin, HIGH);

  cm = duration/29/2;

  return cm;
}

void turn_90_Deg_Left(){
  leftMotorSpeed=0;
  rightMotorSpeed=0;
  set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
  Serial.println("Stop");
  send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
  delay(2000);
  leftEncoder.zero();
  rightEncoder.zero();
  int tickGoal=290;
  leftMotorSpeed=100;
  rightMotorSpeed=100;
  leftMotorRev=1;
  rightMotorRev=0;
  set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
  Serial.println("Turning left");
  send_CAN_Msg(&driveMotorsId,driveMotorsBuf);

  leftEncodRawPos=leftEncoder.getRawPosition();
  rightEncodRawPos=rightEncoder.getRawPosition();
  while((leftEncodRawPos>-tickGoal)||(rightEncodRawPos<tickGoal))
  { 
  if(leftEncodRawPos<-tickGoal){
  Serial.println("left Stop");
  leftMotorSpeed=0;
  set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
  send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
  }
  if(rightEncodRawPos>tickGoal){
    Serial.println("right Stop");
    rightMotorSpeed=0;
    set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
    send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
  }
  leftEncodRawPos=leftEncoder.getRawPosition();
  rightEncodRawPos=rightEncoder.getRawPosition();
  Serial.print("Left encoder raw pos: ");
  Serial.println(leftEncodRawPos);
  Serial.print("Right encoder raw pos: ");
  Serial.println(rightEncodRawPos);
}
}

