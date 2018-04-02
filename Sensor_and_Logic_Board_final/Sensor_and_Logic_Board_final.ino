//include librarys
#include<mcp_can.h>
#include<SPI.h>

//declare pins
const int frontUltrasonicPin=9;
const int SPI_CS_PIN=10;

//create objects
MCP_CAN CAN(SPI_CS_PIN);

//init variables
unsigned int stage=0;
byte huggingArmPos=0;
byte extendArmPos=0;
byte leftMotorSpeed=0;
byte rightMotorSpeed=0;
byte leftMotorRev=0;
byte rightMotorRev=0;
long frontDistance;

//init CAN variables
unsigned long driveMotorsId=0x01;
byte driveMotorsBuf[8]={0,0,0,0,0,0,0,0}; //0=leftMotorSpeed, 1=rightMotorSpeed, 2=leftMotorRev, 3=rightMotorRev
unsigned long miscMotorsId=0x02;
byte miscMotorsBuf[8]={0,0,0,0,0,0,0,0};

//function prototypes
void send_CAN_Msg(unsigned long *msgId,byte *msgBuf);
void set_CAN_TX_Buf(byte *buf,byte *b0=0, byte *b1=0, byte *b2=0, byte *b3=0, byte *b4=0, byte *b5=0, byte *b6=0, byte *b7=0);
long ultrasonicPing(const int ultPin);

void setup() {
Serial.begin(9600);

while(CAN_OK!=CAN.begin(CAN_500KBPS)){
  Serial.println("CAN BUS init fail");
  Serial.println("Init CAN BUS fail again");
  delay(100);
}Serial.println("CAN BUS init ok!");

//Pinmodes

}

void loop() {
switch(stage){
case 0:{ //inital start, drive straight to wall, make left turn

//Turn on front ultrasonic

//Code to drive straight
/***********************************/
leftMotorSpeed=100;
rightMotorSpeed=100;
leftMotorRev=0;
rightMotorRev=0;
/***********************************/

//Code to make a 90 degree left turn
/************************************/


//Increase stage after make a succesful first 90 degree turn

/************************************/

}
case 1:{ //Wall follow code with the obtaining of the tesseract to move on
//Code to follow wall
/******************************/
//Turn on side ultrasonics
//Turn on swinging arm
/*****************************/

//Code to make a 90 dgree left turn after seeing wall
/************************************************/
//If front distance is less than "Enter min front distance here"
//Make left turn function
/**********************************************/

//Code to see if we have a tesseract
/***********************************/
//If halleffect sensor sees the tesseract
//increase stage
/**************************************/
}

case 2:{  //Find pyramid 
  //Stage initialization code
  /*************************************/
//Turn off side ultrasonics
//Turn off swinging arm
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
  /********************************/
}

case 3:{ //Deposit tesseract code
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
huggingArmPos=130;
set_CAN_TX_Buf(miscMotorsBuf,&huggingArmPos,&extendArmPos);
send_CAN_Msg(&miscMotorsId,miscMotorsBuf);

//Delay 2 seconds
delay(2000);

//Extend tipping arm
extendArmPos=180;
set_CAN_TX_Buf(miscMotorsBuf,&huggingArmPos,&extendArmPos);
send_CAN_Msg(&miscMotorsId,miscMotorsBuf);

//Delay 2 seconds
delay(2000);

//Drive backwards
leftMotorSpeed=100;
rightMotorSpeed=100;
leftMotorRev=1;
rightMotorRev=1;
set_CAN_TX_Buf(driveMotorsBuf,&leftMotorSpeed,&rightMotorSpeed,&leftMotorRev,&rightMotorRev);
send_CAN_Msg(&driveMotorsId,driveMotorsBuf);

//retract tipping arm
extendArmPos=0;
set_CAN_TX_Buf(miscMotorsBuf,&huggingArmPos,&extendArmPos);
send_CAN_Msg(&miscMotorsId,miscMotorsBuf);

//Delay 1 second
delay(1000);

//retract hugging arm
huggingArmPos=130;
set_CAN_TX_Buf(miscMotorsBuf,&huggingArmPos,&extendArmPos);
send_CAN_Msg(&miscMotorsId,miscMotorsBuf);

//
//Increase stage
  /**********************/
  stage++;
  /*********************/
}

case 4:{//Code to turn everything off
  /**************************/
//Detatch all motors
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

long ultrasonicPing(const int ultPin){
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

