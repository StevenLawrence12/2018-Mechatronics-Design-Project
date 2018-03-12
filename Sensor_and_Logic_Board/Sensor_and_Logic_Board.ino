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
bool leftDriveMotorReverse;
int rightDriveMotorSpeed=0;
bool rightDriveMotorReverse;

//CAN id's
int motorDrive=0x3;

//CAN messagebufs
byte motorDriveBuf[8];

//checks the distance infront of the robot
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

//leftEncoder.init((/*put ft/motor output shaft rotations*/)*MOTOR_393_SPEED_ROTATIONS,MOTOR_393_TIME_DELTA);
//leftEncoder.setReversed(/*true or false*/);
//rightEncoder.init((/*put ft/motor output shaft rotations*/)*MOTOR_393_SPEED_ROTATIONS,MOTOR_393_TIME_DELTA);
//rightEncoder.setReversed(/*true or false*/);


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
pinMode(sideTrigPin,OUTPUT);
pinMode(sideTrigPin,INPUT);

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

ultrasonicRead(frontTrigPin,frontEchoPin,&frontDistance);  //reads front ultrasonic pin
ultrasonicRead(sideTrigPin,sideEchoPin,&sideDistance); //reads side ultrasonic pin
Serial.print("Front distance");
Serial.println(frontDistance);
if(frontDistance<=frontStopDistance)
{
  motorDriveBuf[0]=0x1;
 CAN.sendMsgBuf(motorDrive,0,8,motorDriveBuf);
}

}
