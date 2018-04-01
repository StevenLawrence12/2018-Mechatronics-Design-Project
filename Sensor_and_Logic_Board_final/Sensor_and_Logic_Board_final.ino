//include librarys
#include<mcp_can.h>
#include<SPI.h>

//declare pins
MCP_CAN CAN(SPI_CS_PIN);

//create objects

//init variables
unsigned int stage=0;

//init CAN variables

//function prototypes
void sendCANMsg(unsigned long *msgId,byte *msgBuf);

void setup() {
Serial.begin(9600);

while(CAN_OK!=CAN.begin(CAN_500KBPS)){
  Serial.println("CAN BUS init fail");
  Serial.println("Init CAN BUS fail again");
  delay(100);
}Serial.println("CAN BUS init ok!");

}

void loop() {
switch(stage){
case 0:{ //inital start, drive straight to wall, make left turn

//Code to drive straight
/***********************************/



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
//Extend tipping arm
//Drive backwards
//retract tipping arm
//retract hugging arm
//Increase stage
  /**********************/
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
