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
unsigned int stage=3;
//other motor/servo variables
byte huggingArmPos=0;
byte extendArmPos=0;
byte swingArmSpeed=0;
//hall effect sensor variables
int hallEffectRead;
int hallEffectMin=528;
int hallEffectMax=540;
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
byte leftDr=0;
byte rightDr=0;
byte swing=0;
byte hug=0;
byte extend=0;

//constants
int HUGARMFINPOS=100;

//init CAN variables
unsigned long driveMotorsId=0x01;
byte driveMotorsBuf[8]={0,0,0,0,0,0,0,0}; //0=leftMotorSpeed, 1=rightMotorSpeed, 2=leftMotorRev, 3=rightMotorRev
unsigned long miscMotorsId=0x02;
byte miscMotorsBuf[8]={0,0,0,0,0,0,0,0}; //0=hug arm position, 1=extending arm position, 2=swinging arm speed;
unsigned long connMotorId=0x03;
byte connMotorBuf[5]={0,0,0,0,0}; //0=left motor on/off, 1=right motor on/off, 2=swinging motor on/off, 3=hugging motor on/off, 4=extending motor on/off

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

while(CAN_OK!=CAN.begin(CAN_500KBPS)){
  Serial.println("CAN BUS init fail");
  Serial.println("Init CAN BUS fail again");
  delay(100);
}Serial.println("CAN BUS init ok!");


}

void loop() {
switch(stage){
case 0:{ //inital start, drive straight to wall, make left turn

//initalize case
/*********************************/
//Turn on drive motors
leftDr=1;
rightDr=1;
set_CAN_TX_Buf(connMotorBuf,leftDr,rightDr,swing,hug,extend);
send_CAN_Msg(&connMotorId,connMotorBuf);

//start driving
leftMotorSpeed=100;
rightMotorSpeed=100;
leftMotorRev=0;
rightMotorRev=0;
set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
/**********************************/
while(stage==0){
  //Code to drive straight
  /***********************************/
  //Drive control with correction
  leftEncodSpeed=leftEncoder.getSpeed();
  rightEncodSpeed=rightEncoder.getSpeed();
  if(leftEncodSpeed>rightEncodSpeed)
  leftMotorSpeed--;
  else if(rightEncodSpeed>leftEncodSpeed)
  leftMotorSpeed++;
  set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
  send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
  /***********************************/
  
  //Check front ultrasonic
  frontDistance=ultrasonic_Ping(frontUltrasonicPin);
  
  //Code to make a 90 degree left turn
  /************************************/
  if(frontDistance<=frontStopDist){
    leftMotorSpeed=0;
    rightMotorSpeed=0;
    set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
    send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
    turn_90_Deg_Left();
    stage++;
  }
/************************************/
}
break;
}
case 1:{ //Wall follow code with the obtaining of the tesseract to move on
  
/******************************/
//Turn on hall effect sensor
pinMode(hallPin,INPUT);
//Turn on swinging arm
swing=1;
set_CAN_TX_Buf(connMotorBuf,leftDr,rightDr,swing,hug,extend);
send_CAN_Msg(&connMotorId,connMotorBuf);
swingArmSpeed=255;
set_CAN_TX_Buf(miscMotorsBuf,&huggingArmPos,&extendArmPos,&swingArmSpeed);
send_CAN_Msg(&miscMotorsId,miscMotorsBuf);
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
  while(frontDistance<=frontStopDist){
    //Make left turn function
    leftMotorRev=1;
    set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
    send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
    
    //check front ultrasonic again
    frontDistance=ultrasonic_Ping(frontUltrasonicPin);
  }
/**********************************************/


//Code to see if we have a tesseract
/***********************************/
//read hall effect sensor
hallEffectRead=analogRead(hallPin);
//If halleffect sensor sees the tesseract
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
  //Stage initialization code
  /*************************************/
//Turn off side ultrasonics
//Turn off swinging arm
swing=0;
set_CAN_TX_Buf(connMotorBuf,leftDr,rightDr,swing,hug,extend);
send_CAN_Msg(&connMotorId,connMotorBuf);
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
  set_CAN_TX_Buf(miscMotorsBuf,&huggingArmPos,&extendArmPos,&swingArmSpeed);
send_CAN_Msg(&miscMotorsId,miscMotorsBuf);
  //Init stage code
  /*********************/
  //Turn off IR sensors
  //Turn off front ultrasonic
  //Turn on hugging arm
  //Turn on tipping arm
  hug=1;
  extend=1;
  set_CAN_TX_Buf(connMotorBuf,&leftDr,&rightDr,&swing,&hug,&extend);
  send_CAN_Msg(&connMotorId,connMotorBuf);
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
  /**************************/
//Detatch all motors
leftDr=0;
rightDr=0;
hug=0;
extend=0;
set_CAN_TX_Buf(connMotorBuf,leftDr,rightDr,swing,hug,extend);
send_CAN_Msg(&connMotorId,connMotorBuf);
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
  leftEncoder.zero();
  rightEncoder.zero();
  int tickGoal=305;
  leftMotorSpeed=100;
  rightMotorSpeed=100;
  leftMotorRev=1;
  rightMotorRev=0;
  set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
  send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
  while((leftEncoder.getRawPosition()>-tickGoal)||(rightEncoder.getRawPosition()<tickGoal))
  { 
  if(leftEncoder.getRawPosition()<-tickGoal)
  leftMotorSpeed=0;
  set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
  send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
  }
  if(rightEncoder.getRawPosition()>tickGoal){
    rightMotorSpeed=0;
    set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
    send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
  }
}

