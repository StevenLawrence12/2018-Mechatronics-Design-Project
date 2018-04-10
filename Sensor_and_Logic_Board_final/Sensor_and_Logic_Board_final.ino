//include librarys
#include<mcp_can.h>
#include<SPI.h>
#include<I2CEncoder.h>
#include<Wire.h>
#include<PID_v1.h>
#include <SoftwareSerial.h>
#include"dfs.h"

#define DIST1 450
#define DIST2 300
#define DIST3 150

//declare pins
const int pyramid_Switch=2;
const int armSwitch=5;
const int sideUlt1Pin=6;
const int sideUlt2Pin=7;
const int frontRightUltPin=8;
const int frontLeftUltPin=9;
const int SPI_CS_PIN=10;
const int hallPin=A3;
const int CASE_SWITCH=A0;


//create objects
MCP_CAN CAN(SPI_CS_PIN);
I2CEncoder leftEncoder;
I2CEncoder rightEncoder;
SoftwareSerial IR_Receiver(3,4);

//init variables
unsigned int stage=0;
//other motor/servo variables
byte huggingArmPos=0;
byte extendArmPos=0;
byte magArmMotorSpeed=0;
byte flickServoPos=20;
//hall effect sensor variables
int hallEffectRead;
int hallEffectMin=500;
int hallEffectMax=520;
//distance variables
unsigned int corFrontStopDist=35;
//Drive Motor variables
byte leftMotorSpeed=0;
byte rightMotorSpeed=0;
byte leftMotorRev=0;
byte rightMotorRev=0;
//ultrasonic variables
long frontRightDist;
long frontLeftDist;
double sideFrontDist;
double sideRearDist;
//Encoder variables
long leftEncodRawPos;
long rightEncodRawPos;
double leftEncodSpeed;
double rightEncodSpeed;
//PID pins
double targSideDist,output1;
double Kp1=11.5,Ki1=0,Kd1=16.5;
double output2;
double Kp2=3,Ki2=0,Kd2=5;

bool foundPyramid = false;

PID sidePID1(&sideFrontDist,&output1,&targSideDist,Kp1,Ki1,Kd1,REVERSE);
PID sidePID2(&sideRearDist,&output2,&sideFrontDist,Kp2,Ki2,Kd2,REVERSE);

int turnCount=0;

//mapping stuff
long currPos[2];
long corner1[2]={0,0};
long corner2[2];
long corner3[2];
long corner4[2];

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
void turn_Left(double degree);
void mapping(int turnCount);
void adjust_Back(long frontRightDist,long frontLeftDist);
void getUnstuck();


void setup() {
Serial.begin(9600);
Wire.begin();
IR_Receiver.begin(2400);
randomSeed(analogRead(0));

targSideDist=TARGET_SIDE_DIST;

sidePID1.SetMode(AUTOMATIC);
sidePID2.SetMode(AUTOMATIC);
sidePID1.SetOutputLimits(-40,40);
sidePID2.SetOutputLimits(-40,40);

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

set_CAN_TX_Buf(miscMotorsBuf,&huggingArmPos,&extendArmPos,&magArmMotorSpeed, &flickServoPos);
send_CAN_Msg(&miscMotorsId,miscMotorsBuf);

pinMode(hallPin,INPUT);
pinMode(armSwitch,INPUT);
pinMode(pyramid_Switch,INPUT);
pinMode(CASE_SWITCH,INPUT);

}

void loop() {
switch(stage){
case 0:{ //inital start, drive straight to wall, make left turn
Serial.println("Stage 0");
//initalize case
/*********************************/
//Turn on drive motors

//start driving
leftMotorSpeed=DEFAULT_LEFT_SPEED;
rightMotorSpeed=DEFAULT_RIGHT_SPEED;
leftMotorRev=FORWARD;
rightMotorRev=FORWARD;
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
  send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
  /***********************************/
  
  //Check front ultrasonic
  frontRightDist=ultrasonic_Ping(frontRightUltPin);
  frontLeftDist=ultrasonic_Ping(frontLeftUltPin);
  Serial.print("Front Right distance: ");
  Serial.println(frontRightDist);
  Serial.print("Front Left distance: ");
  Serial.println(frontLeftDist);
  //Code to make a 90 degree left turn
  /************************************/
  if(frontRightDist<FRONT_STOP_DIST){
rightMotorSpeed=STOP_SPEED;
set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
  }
  if(frontLeftDist<FRONT_STOP_DIST){
leftMotorSpeed=STOP_SPEED;
set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
send_CAN_Msg(&driveMotorsId,driveMotorsBuf);    
  }
  if(frontRightDist<=FRONT_STOP_DIST&&frontLeftDist<=FRONT_STOP_DIST){
    //adjust_Back(frontRightDist,frontLeftDist);
    turn_Left(90);
    leftEncoder.zero();
    rightEncoder.zero();
    stage++;
  }
/************************************/
}
break;
}


case 1:{ //Wall follow code with the obtaining of the tesseract to move on
/******************************/
leftMotorSpeed=STOP_SPEED;
rightMotorSpeed=STOP_SPEED;
set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
delay(1000);
/*****************************/

while(stage==1){
Serial.println("Stage 1");
//Init stage 1

//Code to follow wall
/******************************************/
//check side ultrasonics
sideFrontDist=ultrasonic_Ping(sideUlt1Pin);
sideRearDist=ultrasonic_Ping(sideUlt2Pin);
Serial.print("side dist 1: ");
Serial.println(sideFrontDist);
Serial.print("side dist 2: ");
Serial.println(sideRearDist);

//compare side distances
bool one=sidePID1.Compute();
leftMotorSpeed=DEFAULT_LEFT_SPEED+output1;
Serial.print("Output 1: ");
Serial.println(output1);

if(sideFrontDist>7&&sideRearDist<=8){
  bool two=sidePID2.Compute();
  leftMotorSpeed=DEFAULT_RIGHT_SPEED-output2;
  Serial.print("Output 2: ");
  Serial.println(output2);
}

//}

leftMotorRev=FORWARD;
rightMotorRev=FORWARD;
set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
//make proper driving correction
/*****************************************/

//Check front ultrasonic
  frontRightDist=ultrasonic_Ping(frontRightUltPin);
  frontLeftDist=ultrasonic_Ping(frontLeftUltPin);
  Serial.print("Front Right distance: ");
  Serial.println(frontRightDist);
  Serial.print("Front Left distance: ");
  Serial.println(frontLeftDist);
  //Code to make a 90 degree left turn
  /************************************/
  if(frontRightDist<FRONT_STOP_DIST){
rightMotorSpeed=STOP_SPEED;
set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
  }
  if(frontLeftDist<FRONT_STOP_DIST){
leftMotorSpeed=STOP_SPEED;
set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
send_CAN_Msg(&driveMotorsId,driveMotorsBuf);    
  }
  if(frontRightDist<=FRONT_STOP_DIST&&frontLeftDist<=FRONT_STOP_DIST){
    //adjust_Back(frontRightDist,frontLeftDist);
    turn_Left(90);
    turnCount++;
    leftEncoder.zero();
    rightEncoder.zero();
    leftMotorSpeed=DEFAULT_LEFT_SPEED;
rightMotorSpeed=DEFAULT_RIGHT_SPEED;
leftMotorRev=FORWARD;
rightMotorRev=FORWARD;
set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
  }

/*leftEncodSpeed=leftEncoder.getSpeed()/60;
  rightEncodSpeed=rightEncoder.getSpeed()/60;
  if((leftEncodSpeed<STUCK_SPEED)&&(leftMotorSpeed!=STOP_SPEED)){
       getUnstuck();
   leftMotorSpeed=DEFAULT_LEFT_SPEED;
   rightMotorSpeed=DEFAULT_RIGHT_SPEED;
   leftMotorRev=FORWARD;
   rightMotorRev=FORWARD;
   set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
   send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
  }
  else if((rightEncodSpeed<STUCK_SPEED)&&(rightMotorSpeed!=STOP_SPEED)){
    getUnstuck();
    leftMotorSpeed=DEFAULT_LEFT_SPEED;
   rightMotorSpeed=DEFAULT_RIGHT_SPEED;
   leftMotorRev=FORWARD;
   rightMotorRev=FORWARD;
   set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
   send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
  }*/
/**********************************************/


//Code to see if we have a tesseract
/***********************************/
if(turnCount==1){

/*************************/
//pull tesseract in code here
magArmMotorSpeed=MAGNET_ARM_RETRACT_SPEED;
set_CAN_TX_Buf(miscMotorsBuf,&huggingArmPos,&extendArmPos,&magArmMotorSpeed, &flickServoPos);
send_CAN_Msg(&miscMotorsId,miscMotorsBuf);

/***************************/

if(digitalRead(armSwitch)==HIGH){
  Serial.println("Pulling");
  magArmMotorSpeed=STOP_SPEED;
  set_CAN_TX_Buf(miscMotorsBuf,&huggingArmPos,&extendArmPos,&magArmMotorSpeed,&flickServoPos);
  send_CAN_Msg(&miscMotorsId,miscMotorsBuf);
  delay(1500); 
  flickServoPos=40;
  set_CAN_TX_Buf(miscMotorsBuf,&huggingArmPos,&extendArmPos,&magArmMotorSpeed,&flickServoPos);
  send_CAN_Msg(&miscMotorsId,miscMotorsBuf);
  delay(200);
  flickServoPos=60;
  set_CAN_TX_Buf(miscMotorsBuf,&huggingArmPos,&extendArmPos,&magArmMotorSpeed,&flickServoPos);
  send_CAN_Msg(&miscMotorsId,miscMotorsBuf);
  delay(200);
  flickServoPos=80;
  set_CAN_TX_Buf(miscMotorsBuf,&huggingArmPos,&extendArmPos,&magArmMotorSpeed,&flickServoPos);
  send_CAN_Msg(&miscMotorsId,miscMotorsBuf);
  delay(200);
  flickServoPos=100;
  set_CAN_TX_Buf(miscMotorsBuf,&huggingArmPos,&extendArmPos,&magArmMotorSpeed,&flickServoPos);
  send_CAN_Msg(&miscMotorsId,miscMotorsBuf);
  delay(200);
  flickServoPos=120;
  set_CAN_TX_Buf(miscMotorsBuf,&huggingArmPos,&extendArmPos,&magArmMotorSpeed,&flickServoPos);
  send_CAN_Msg(&miscMotorsId,miscMotorsBuf);
  delay(200);
  flickServoPos=140;
  set_CAN_TX_Buf(miscMotorsBuf,&huggingArmPos,&extendArmPos,&magArmMotorSpeed,&flickServoPos);
  send_CAN_Msg(&miscMotorsId,miscMotorsBuf);
  delay(200);
  flickServoPos=160;
  set_CAN_TX_Buf(miscMotorsBuf,&huggingArmPos,&extendArmPos,&magArmMotorSpeed,&flickServoPos);
  send_CAN_Msg(&miscMotorsId,miscMotorsBuf);
  delay(200);
  
  stage++;
}

/*************************************************/
//Add code here to reextend magnet arm

//else {
  
//}

/*************************************************/
/*for(int i=0, hallConsecRead=0; i<20; i++){
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
}*/

}
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
while(stage==2){
  
//turn_Left(120);
  /*************************************/
  
  //Robot movement path code
  /*****************************/
/*leftMotorSpeed=DEFAULT_SPEED;
rightMotorSpeed=DEFAULT_SPEED;
leftMotorRev=FORWARD;
rightMotorRev=FORWARD;
set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
send_CAN_Msg(&driveMotorsId,driveMotorsBuf);

frontRightDist=ultrasonic_Ping(frontRightUltPin);
  frontLeftDist=ultrasonic_Ping(frontLeftUltPin);
  Serial.print("Front Right distance: ");
  Serial.println(frontRightDist);
  Serial.print("Front Left distance: ");
  Serial.println(frontLeftDist);
  //Code to make a 90 degree left turn
  /************************************/

  if (foundPyramid == true){
      if (IR_Receiver.available()){
        leftMotorSpeed = DEFAULT_SPEED;
        rightMotorSpeed = DEFAULT_SPEED;
        leftMotorRev=FORWARD;
        rightMotorRev=FORWARD;
        set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
   send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
      } else {
        leftMotorSpeed = DEFAULT_SPEED;
        rightMotorSpeed = DEFAULT_SPEED;
        leftMotorRev=BACKWARD;
        rightMotorRev=FORWARD;
        set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
   send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
      }
      if (pyramid_Switch == 1) stage++;
    }
    else {
      if (!(IR_Receiver.available())){
        int randAngle = random(249,301);
        turn_Left(randAngle);
        leftEncoder.zero();
        rightEncoder.zero();
        while(leftEncoder.getRawPosition()<1425&&rightEncoder.getRawPosition()<1425)
        {
          leftMotorSpeed=DEFAULT_LEFT_SPEED;
   rightMotorSpeed=DEFAULT_RIGHT_SPEED;
   leftMotorRev=FORWARD;
   rightMotorRev=FORWARD;
   set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
   send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
          if(leftEncoder.getRawPosition()>1425){
            leftMotorSpeed=STOP_SPEED;
  set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
  send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
          }
          if(rightEncoder.getRawPosition()>1425){
            rightMotorSpeed=STOP_SPEED;
  set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
  send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
          }
        }
        //move forward random distance between 50 and 100 cm with checking surroundings
        
      } else {    
        char reading = IR_Receiver.read();
        if (CASE_SWITCH == 1){     //case switch 1 == AE 
          if (reading == 'A' || reading == 'E'){
            foundPyramid = true;
          } else if (reading == 'I' || reading == 'O'){
          } else {
            leftMotorSpeed == DEFAULT_SPEED;
            rightMotorSpeed == DEFAULT_SPEED;
          }
        }
        if (CASE_SWITCH == 0){     //case switch 0 == IO
          if (reading == 'I' || reading == 'O'){
            foundPyramid = true;
          } else if (reading == 'A' || reading == 'E'){
          } else {
            leftMotorSpeed == DEFAULT_SPEED;
            rightMotorSpeed == DEFAULT_SPEED;
          }
        }         
      }
    }

    
  if(frontRightDist<FRONT_STOP_DIST35){
rightMotorSpeed=STOP_SPEED;
set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
  }
  if(frontLeftDist<FRONT_STOP_DIST35){
leftMotorSpeed=STOP_SPEED;
set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
send_CAN_Msg(&driveMotorsId,driveMotorsBuf);    
  }
  if(frontRightDist<=FRONT_STOP_DIST&&frontLeftDist<=FRONT_STOP_DIST){
    turn_Left(120);
    leftEncoder.zero();
    rightEncoder.zero();
  }

  /****************************/

  //Code after pyramid is seen
  /*********************************/

/*if(IR_Receiver.available()){
  
}
//If IR receiver sees pyramid signal
//Stop
//Drive straight

//Increase stage
stage++;
  /********************************/
}/*
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
set_CAN_TX_Buf(miscMotorsBuf,&huggingArmPos,&extendArmPos,&magArmMotorSpeed);
send_CAN_Msg(&miscMotorsId,miscMotorsBuf);

//Delay 2 seconds
delay(2000);

//Extend tipping arm
extendArmPos=160;
set_CAN_TX_Buf(miscMotorsBuf,&huggingArmPos,&extendArmPos,&magArmMotorSpeed);
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
    leftMotorSpeed=STOP_SPEED;
    rightMotorSpeed=DEFAULT_SPEED;
    leftMotorRev=BACKWARD;
    rightMotorRev=BACKWARD;
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
set_CAN_TX_Buf(miscMotorsBuf,&huggingArmPos,&extendArmPos,&magArmMotorSpeed);
send_CAN_Msg(&miscMotorsId,miscMotorsBuf);

//Delay 1 second
delay(1000);

//retract hugging arm
huggingArmPos=0;
set_CAN_TX_Buf(miscMotorsBuf,&huggingArmPos,&extendArmPos,&magArmMotorSpeed);
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

void turn_Left(double degree){
  leftMotorSpeed=STOP_SPEED;
  rightMotorSpeed=STOP_SPEED;
  set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
  Serial.println("Stop");
  send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
  delay(1500);
  leftEncoder.zero();
  rightEncoder.zero();
  double tickGoal=(degree*100)/29;
  leftMotorSpeed=DEFAULT_TURN_SPEED;
  rightMotorSpeed=DEFAULT_TURN_SPEED;
  leftMotorRev=BACKWARD;
  rightMotorRev=FORWARD;
  set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
  Serial.println("Turning left");
  send_CAN_Msg(&driveMotorsId,driveMotorsBuf);

  leftEncodRawPos=leftEncoder.getRawPosition();
  rightEncodRawPos=rightEncoder.getRawPosition();
  while((leftEncodRawPos>-tickGoal)||(rightEncodRawPos<tickGoal))
  { 
   leftEncodSpeed=leftEncoder.getSpeed()/60;
  rightEncodSpeed=rightEncoder.getSpeed()/60;
  
  /*if((leftEncodSpeed<STUCK_SPEED)&&(leftMotorSpeed!=STOP_SPEED)){
       getUnstuck();
   leftMotorSpeed=DEFAULT_LEFT_SPEED;
   rightMotorSpeed=DEFAULT_RIGHT_SPEED;
   leftMotorRev=FORWARD;
   rightMotorRev=FORWARD;
   set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
   send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
  }
  else if((rightEncodSpeed<STUCK_SPEED)&&(rightMotorSpeed!=STOP_SPEED)){
    getUnstuck();
    leftMotorSpeed=DEFAULT_LEFT_SPEED;
   rightMotorSpeed=DEFAULT_RIGHT_SPEED;
   leftMotorRev=FORWARD;
   rightMotorRev=FORWARD;
   set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
   send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
  }*/
  
  leftEncodRawPos=leftEncoder.getRawPosition();
  rightEncodRawPos=rightEncoder.getRawPosition();
  
  if(leftEncodRawPos<-tickGoal){
  Serial.println("left Stop");
  leftMotorSpeed=STOP_SPEED;
  set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
  send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
  }
  if(rightEncodRawPos>tickGoal){
    Serial.println("right Stop");
    rightMotorSpeed=STOP_SPEED;
    set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
    send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
  }
  /*Serial.print("Left encoder raw pos: ");
  Serial.println(leftEncodRawPos);
  Serial.print("Right encoder raw pos: ");
  Serial.println(rightEncodRawPos);*/
}

/*while(true){
long sideFrontDist=ultrasonic_Ping(sideUlt1Pin);
long sideRearDist=ultrasonic_Ping(sideUlt2Pin);
if(sideFrontDist-sideRearDist>=3){
  leftMotorSpeed=DEFAULT_TURN_SPEED;
  rightMotorSpeed=DEFAULT_TURN_SPEED;
  leftMotorRev=FORWARD;
  rightMotorRev=BACKWARD;
  set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
  send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
}
else if (sideRearDist-sideFrontDist>=3){
  leftMotorSpeed=DEFAULT_TURN_SPEED;
  rightMotorSpeed=DEFAULT_TURN_SPEED;
  leftMotorRev=BACKWARD;
  rightMotorRev=FORWARD;
  set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
  send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
}
else {
  leftMotorSpeed=STOP_SPEED;
  rightMotorSpeed=STOP_SPEED;
  set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
  send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
  break;
}
}

}*/
}

void mapping(int turnCount){
  
  if(turnCount==0){
    currPos[0]=0;
    currPos[1]=0;
  }
    
    if(turnCount==1){
    long leftTick=leftEncoder.getRawPosition();
    long rightTick=rightEncoder.getRawPosition();
    if(leftTick>rightTick)
    currPos[0]=rightTick;
    else if(rightTick>leftTick)
    currPos[0]=leftTick;
    else currPos[0]=rightTick;
    corner2[0]=currPos[0];
    corner2[1]=currPos[1];
    }
    
   else if(turnCount==2){
    long leftTick=leftEncoder.getRawPosition();
    long rightTick=rightEncoder.getRawPosition();
    if(leftTick>rightTick)
    currPos[1]=rightTick;
    else if(rightTick>leftTick)
    currPos[1]=leftTick;
    else currPos[1]=rightTick;
    corner3[0]=currPos[0];
    corner3[1]=currPos[1];
    }

    else if(turnCount==3){
    long leftTick=leftEncoder.getRawPosition();
    long rightTick=rightEncoder.getRawPosition();
    if(leftTick>rightTick)
    currPos[0]=-rightTick;
    else if(rightTick>leftTick)
    currPos[0]=-leftTick;
    else currPos[1]=-rightTick;
    corner4[0]=currPos[0];
    corner4[1]=currPos[1];
    }
    
    else if(turnCount==4){
    long leftTick=leftEncoder.getRawPosition();
    long rightTick=rightEncoder.getRawPosition();
    if(leftTick>rightTick)
    currPos[1]=-rightTick;
    else if(rightTick>leftTick)
    currPos[1]=-leftTick;
    else currPos[1]=-rightTick;
    }

  
  Serial.print("Current Position: ");
    for(int i=0; i<2; i++){
      Serial.print(currPos[i]);
      Serial.print(", ");
    }Serial.println();
}

void adjust_Back(long frontRightDist,long frontLeftDist){

  Serial.println("adjust");

  leftEncoder.zero();
  rightEncoder.zero();

  double leftDist=TOO_CLOSE-frontLeftDist;
  double rightDist=TOO_CLOSE-frontRightDist;

  double l_Tick_Goal=leftDist*360*200/59/31.9186;
  double r_Tick_Goal=rightDist*360*200/59/31.9186;

  leftEncodRawPos=leftEncoder.getRawPosition();
  rightEncodRawPos=rightEncoder.getRawPosition();

  if(leftDist>1)
  leftMotorSpeed=DEFAULT_SPEED;
  else {
    leftMotorSpeed=STOP_SPEED;
    l_Tick_Goal=-1;
  }

  if(rightDist>1)
  rightMotorSpeed=DEFAULT_SPEED;
  else {
    rightMotorSpeed=STOP_SPEED;
    r_Tick_Goal=-1;
  }
  
  leftMotorRev=BACKWARD;
  rightMotorSpeed=BACKWARD;
  set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
  send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
  
  while((leftEncoder.getRawPosition()>-l_Tick_Goal)||(rightEncoder.getRawPosition()>-r_Tick_Goal))
  {
  if(leftEncoder.getRawPosition()<-l_Tick_Goal){
  Serial.println("left Stop");
  leftMotorSpeed=STOP_SPEED;
  set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
  send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
  }
  if(rightEncoder.getRawPosition()<-r_Tick_Goal){
    Serial.println("right Stop");
    rightMotorSpeed=STOP_SPEED;
    set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
    send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
  }
  leftMotorSpeed=STOP_SPEED;
  rightMotorSpeed=STOP_SPEED;
  set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
  send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
}
}

void getUnstuck(){
  Serial.println("Stuck");
  long lastLeftTick=leftEncoder.getRawPosition();
  long lastRightTick=rightEncoder.getRawPosition();

  long currLeftTick=leftEncoder.getRawPosition();
  long currRightTick=rightEncoder.getRawPosition();
  
  while(lastLeftTick-currLeftTick<UNSTUCK_TICK||lastRightTick-currRightTick<UNSTUCK_TICK){
  leftMotorSpeed=LEFT_UNSTUCK_SPEED;
  rightMotorSpeed=RIGHT_UNSTUCK_SPEED;
  leftMotorRev=BACKWARD;
  rightMotorRev=BACKWARD;
  set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
  send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
  Serial.print("Left motor speed: ");
  Serial.println(leftMotorSpeed);
  Serial.print("Right motor speed: ");
  Serial.println(rightMotorSpeed);
  leftEncodSpeed=leftEncoder.getSpeed()/60;
  rightEncodSpeed=rightEncoder.getSpeed()/60;
  Serial.print("Left Speed: ");
  Serial.println(leftEncodSpeed);
  Serial.print("Right Speed: ");
  Serial.println(rightEncodSpeed);
  currLeftTick=leftEncoder.getRawPosition();
  currRightTick=rightEncoder.getRawPosition();
  }
  leftEncoder.zero();
  rightEncoder.zero();
}

