#include<Servo.h>
#include<Wire.h>
#include<I2CEncoder.h>

Servo servo_Motor1;
Servo servo_Motor2;

I2CEncoder encoder1;
I2CEncoder encoder2;

const int MotorPin1=8;
const int MotorPin2=9;

int motor1Speed=1650;
int motor2Speed=1650;

int encoder1Speed;  
int encoder2Speed;

void setup() 
{
  Serial.begin(9600);
  Wire.begin();
  pinMode(MotorPin1,OUTPUT);
  servo_Motor1.attach(MotorPin1);
  pinMode(MotorPin2,OUTPUT);
  servo_Motor2.attach(MotorPin2);

  encoder1.init((10.3)*MOTOR_393_SPEED_ROTATIONS,MOTOR_393_TIME_DELTA);
  encoder1.setReversed(false);
  encoder2.init((10.3)*MOTOR_393_SPEED_ROTATIONS,MOTOR_393_TIME_DELTA);
  encoder2.setReversed(true);
  
}

void loop()
{
  servo_Motor1.writeMicroseconds(motor1Speed);
  servo_Motor2.writeMicroseconds(motor2Speed);

  encoder1Speed=encoder1.getSpeed();
  Serial.print("Speed1: ");
  Serial.print((encoder1Speed/60));
  Serial.println(" cm/s");

  encoder2Speed=encoder2.getSpeed();
  Serial.print("Speed2: ");
  Serial.print((encoder2Speed/60));
  Serial.println(" cm/s");

  if((encoder1Speed-encoder2Speed)>.5)
  {
    motor1Speed--;
  }
  if((encoder2Speed-encoder1Speed)>.5)
  {
    motor1Speed++;
  }
  
}
