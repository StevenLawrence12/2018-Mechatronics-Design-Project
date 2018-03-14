#include<Servo.h> //M
#include<Wire.h>  //S
#include<I2CEncoder.h>  //S

Servo servo_Motor1; //M
Servo servo_Motor2; //M

I2CEncoder encoder1;  //S
I2CEncoder encoder2;  //S

const int MotorPin1=8;  //M
const int MotorPin2=9;  //M

int motor1Speed=1650; //S
int motor2Speed=1650; //S

int encoder1Speed;  //S
int encoder2Speed;  //S

void setup() 
{
  Serial.begin(9600); //B
  Wire.begin(); //S
  pinMode(MotorPin1,OUTPUT);  //M
  servo_Motor1.attach(MotorPin1); //M
  pinMode(MotorPin2,OUTPUT);  //M
  servo_Motor2.attach(MotorPin2); //M

  encoder1.init((10.3)*MOTOR_393_SPEED_ROTATIONS,MOTOR_393_TIME_DELTA); //S
  encoder1.setReversed(false);  //S
  encoder2.init((10.3)*MOTOR_393_SPEED_ROTATIONS,MOTOR_393_TIME_DELTA); //S
  encoder2.setReversed(true); //S
  
}

void loop()
{
  servo_Motor1.writeMicroseconds(motor1Speed);  //M
  servo_Motor2.writeMicroseconds(motor2Speed);  //M

  encoder1Speed=encoder1.getSpeed();  //S
  Serial.print("Speed1: ");
  Serial.print((encoder1Speed/60));
  Serial.println(" cm/s");

  encoder2Speed=encoder2.getSpeed();  //S
  Serial.print("Speed2: ");
  Serial.print((encoder2Speed/60));
  Serial.println(" cm/s");

  if((encoder1Speed-encoder2Speed)>.5) //S
  {
    motor1Speed--;
  }
  if((encoder2Speed-encoder1Speed)>.5)  //S
  {
    motor1Speed++;
  }
  
}
