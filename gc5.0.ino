#include <Servo.h>
#include "LedControl.h"  
#define grayScale1 A2
#define grayScale2 A3
#define motor1L 5
#define motor1R 6
#define motor2L 9
#define motor2R 10
////右轮
//#define motor3 11
////左轮
//#define motor4 3


#define servo1 4
#define encoderTime 50

// 常量
int SPEED = 200;
int theta = 18; // 左右轮速度差
int deltaPWM = 190;

//初始化目标速度
int expectedPWML = SPEED - theta;
int expectedPWMR = SPEED;
//int Expectedspeed = 

//采集到黑线时，传感器为低电平
int gS1 = 1;
int gS2 = 1;
int thresholdR = 160; // 180 170
int thresholdL = 130; // 120
LedControl lc=LedControl(12,11,13,1);

void setup() {
  // put your setup code here, to run once:
  pinMode(grayScale1,INPUT);
  pinMode(grayScale2,INPUT);
  pinMode(motor1L,OUTPUT);
  pinMode(motor1R,OUTPUT); 
  pinMode(motor2L,OUTPUT);
  pinMode(motor2R,OUTPUT);
//  pinMode(motor3,OUTPUT);
//  pinMode(motor4,OUTPUT);
  //MsTimer2::set(encoderTime, pidMotorControl);
 //MsTimer2::start();
  lc.shutdown(0,false);
  lc.setIntensity(0,8);
  lc.clearDisplay(0);
  //一开始匀加速
  //uniformSpeedUp();

  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  pidMotorControl();
//  Serial.print(analogRead(grayScale1));
//  Serial.print(",");
//  Serial.println(analogRead(grayScale2));
  delay(40);
}

void motorRun(int left, int right)
{
  left = left > 255 ? 255 : left;
  left = left <= 0 ? 0 : left;
  right = right > 255 ? 255 : right;
  right = right <= 0 ? 0 : right;
  analogWrite(motor1L,left);
  analogWrite(motor2R,right);
//  analogWrite(motor3,left);
//  analogWrite(motor4,right);
}

//void uniformSpeedUp()
//{
//  for(int i=0;i<=20;i++)
//  {
//    analogWrite(motor1L,10*i);
//    analogWrite(motor2R,10*i);
//    delay(50);
//  }
//  //digitalWrite(motor3,HIGH);
//  //digitalWrite(motor4,HIGH);
//}

void turnRight(){
  lc.setRow(0,0,B00010000);
  lc.setRow(0,1,B11111111);
  lc.setRow(0,2,B00100000);
  lc.setRow(0,3,B00100000);
  lc.setRow(0,4,B01111110);
  lc.setRow(0,5,B11100010);
  lc.setRow(0,6,B11100010);
  lc.setRow(0,7,B01111110);
}

void turnLeft(){
  lc.setRow(0,0,B00010000);
  lc.setRow(0,1,B11111111);
  lc.setRow(0,2,B00100000);
  lc.setRow(0,3,B00100010);
  lc.setRow(0,4,B01111110);
  lc.setRow(0,5,B11001000);
  lc.setRow(0,6,B10001000);
  lc.setRow(0,7,B01111111);
}

void forward(){
  lc.setRow(0,0,B00000000);
  lc.setRow(0,1,B00111100);
  lc.setRow(0,2,B00100000);
  lc.setRow(0,3,B00111100);
  lc.setRow(0,4,B00111100);
  lc.setRow(0,5,B00100000);
  lc.setRow(0,6,B00100000);
  lc.setRow(0,7,B00000000);
}

void down(){
  lc.setRow(0,0,B00000000);
  lc.setRow(0,1,B00111100);
  lc.setRow(0,2,B00100100);
  lc.setRow(0,3,B00100100);
  lc.setRow(0,4,B00100100);
  lc.setRow(0,5,B00100100);
  lc.setRow(0,6,B00111000);
  lc.setRow(0,7,B00000000);
}

void pidMotorControl()
{
  gS1 = analogRead(grayScale1);
  gS2 = analogRead(grayScale2);
  gS1 = gS1>thresholdL?1:0;
  gS2 = gS2>thresholdR?1:0;
  //直行
    if(gS1==1&&gS2==1){
    motorRun(expectedPWML,expectedPWMR);
    forward();
    }
  //左转
  if(gS1==0&&gS2==1){
    motorRun(expectedPWML-deltaPWM,expectedPWMR+deltaPWM);
    turnLeft();
    }
  //右转
  if(gS1==1&&gS2==0){
    motorRun(expectedPWML+deltaPWM,expectedPWMR-deltaPWM);
    turnRight();
    }
  //爬坡和爬楼梯
  if(gS1==0&&gS2==0){
    motorRun(expectedPWML-20,expectedPWMR+10);
    down();
  }
}
