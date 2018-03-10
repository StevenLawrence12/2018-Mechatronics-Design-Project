/*
 * Sensor and Logic Board V1.0
 */
//Include librarys
#include<Wire.h>
#include<I2CEncoder.h>

//create objects
I2CEncoder leftEncoder;
I2CEncoder rightEncoder;

//init variables
double leftEncoderSpeed;
double rightEncoderSpeed;
void setup() 
{ 
Wire.begin();
Serial.begin(9600);

//init encoders 
leftEncoder.init((/*put ft/motor output shaft rotations*/)*MOTOR_393_SPEED_ROTATIONS,MOTOR_393_TIME_DELTA);
leftEncoder.setReversed(/*true or false*/);
rightEncoder.init((/*put ft/motor output shaft rotations*/)*MOTOR_393_SPEED_ROTATIONS,MOTOR_393_TIME_DELTA);
rightEncoder.setReversed(/*true or false*/);
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
}
