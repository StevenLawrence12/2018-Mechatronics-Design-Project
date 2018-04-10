/*
 *Motor Control V2.0
 *
 */
//include librarys
#include<Servo.h>
#include<mcp_can.h>
#include<SPI.h>

//declare pins
const int magMotorPin=2;
const int hugMotorPin=6;
const int tipMotorPin=4;
const int flickServoPin=5;
const int LeftMotorPin=8;
const int RightMotorPin=9;
const int SPI_CS_PIN=10;

//create objects  
MCP_CAN CAN(SPI_CS_PIN);
Servo servo_LeftMotor;
Servo servo_RightMotor;
Servo servo_Magnet;
Servo servo_Hugging;
Servo servo_Extending;
Servo servo_Flicking;

//CAN buf variables
byte leftSpeed=0;
byte rightSpeed=0;
byte leftRev=0;
byte rightRev=0;

byte hugPos=0;
byte armPos=0;
byte magSpeed=0;
byte flickPos=0;

//init variables
unsigned int leftMotorSpeed=0;
unsigned int rightMotorSpeed=0;
unsigned int hugArmPos=0;
unsigned int extArmPos=0;
unsigned int magArmSpeed=0;

//CAN variables
byte len=0;
byte receiveBuf[8];
unsigned long canId;
byte driveBuf[8]={0,
0,0,0,0,0,0,0}; //0=leftMotorSpeed, 1=rightMotorSpeed, 2=leftMotorRev, 3=rightMotorRev
byte miscBuf[8]={0,0,0,0,0,0,0,0}; //0=hug arm position, 1=extending arm position, 2=swinging arm speed, 3=flicking servo position

//drive function
void drive(int leftSpeed,int rightSpeed){
  servo_LeftMotor.writeMicroseconds(leftSpeed);
  servo_RightMotor.writeMicroseconds(rightSpeed);
}

void setup() {
  Serial.begin(9600);

  //pinmodes 
  pinMode(tipMotorPin,OUTPUT);
  pinMode(magMotorPin,OUTPUT);
  pinMode(hugMotorPin,OUTPUT);
  pinMode(LeftMotorPin,OUTPUT);
  pinMode(RightMotorPin,OUTPUT);
  pinMode(flickServoPin,OUTPUT);

  servo_Hugging.attach(hugMotorPin);
  servo_Extending.attach(tipMotorPin);
  servo_LeftMotor.attach(LeftMotorPin);
  servo_RightMotor.attach(RightMotorPin);
  servo_Magnet.attach(magMotorPin);
  servo_Flicking.attach(flickServoPin);

  //init CAN
  while(CAN_OK!=CAN.begin(CAN_1000KBPS)){
    Serial.println("CAN BUS init fail");
    Serial.println("Init CAN BUS fail again");
    delay(100);
  }Serial.println("CAN BUS init ok!"); 
  //CAN.begin(CAN_1000KBPS);
}

void loop() {
  //receiving CAN message
  if(CAN_MSGAVAIL==CAN.checkReceive()) //checks if there is anything to read in CAN buffers
  {
    Serial.println("receive");
    CAN.readMsgBuf(&len, receiveBuf); //reading CAN message
    canId=CAN.getCanId();  //read the message id associated with this CAN message

  Serial.print("CAN ID: ");
  Serial.println(canId);

  for(int i=0; i<8;i++){
    Serial.print(receiveBuf[i]);
    Serial.print(", ");
  }Serial.println();
    if(canId==0x01){
    for(int i=0;i<8;i++){
      driveBuf[i]=receiveBuf[i];
    }
    }
    else if(canId==0x02){
      for(int i=0;i<8;i++){
      miscBuf[i]=receiveBuf[i];
    }
    }
  }

  hugArmPos=miscBuf[0];
  extArmPos=miscBuf[1];
  /*Serial.println(hugArmPos);
  Serial.println(extArmPos);*/
  magArmSpeed=miscBuf[2];
  flickPos=miscBuf[3];
  
  //set drive motor speeds
  if(driveBuf[2]==1) leftMotorSpeed=1500-driveBuf[0];
  else leftMotorSpeed=1500+driveBuf[0];
  if(driveBuf[3]==1) rightMotorSpeed=1500-driveBuf[1];
  else rightMotorSpeed=1500+driveBuf[1];

  /*Serial.println(leftMotorSpeed);
  Serial.println(rightMotorSpeed);*/

  //drive motors
  drive(leftMotorSpeed, rightMotorSpeed);
  //Serial.println(hugArmPos);
  servo_Hugging.write(hugArmPos);
  servo_Extending.write(extArmPos);
  servo_Magnet.writeMicroseconds(magArmSpeed+1500);
  servo_Flicking.write(flickPos);
  
  
}
