/********************
 * 2022.5.29 18:23 编辑，加入颜色识别模块
 */
#include <Servo.h>
#include "LedControl.h"  
#include <Wire.h>        //调用IIC库函数
#include "MH_TCS34725.h" //调用颜色识别传感器库函数
#ifdef __AVR__
#include <avr/power.h>
#endif
#define grayScale1 A2
#define grayScale2 A3
#define motor1L 5
#define motor1R 6
#define motor2L 9
#define motor2R 10
#define SERVO 7
//#define encoderTime 50

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
int thresholdL = 140; // 120
LedControl lc=LedControl(12,11,13,1);

// 颜色识别相关
// 颜色阈值
uint32_t thresh_red = 1800;
uint32_t thresh_green = 1400;
uint32_t thresh_blue = 1900;
// 颜色值
uint16_t red, green, blue, sum;
float r, g, b;
uint8_t color = 0, card_color = 0;
// 颜色传感器
MH_TCS34725 tcs = MH_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X); // 50ms
// MH_TCS34725 tcs = MH_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);//700ms

// 初始化舵机
Servo my_servo;
// 舵机位置
uint8_t DOWN = 0, UP = 180;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(grayScale1,INPUT);
  pinMode(grayScale2,INPUT);
  pinMode(motor1L,OUTPUT);
  pinMode(motor1R,OUTPUT); 
  pinMode(motor2L,OUTPUT);
  pinMode(motor2R,OUTPUT);
  //MsTimer2::set(encoderTime, pidMotorControl);
 //MsTimer2::start();
  lc.shutdown(0,false);
  lc.setIntensity(0,8);
  lc.clearDisplay(0);
  //一开始匀加速
  //uniformSpeedUp();
  if (!tcs.begin())
  {
    while(1);
  }
  // 舵机初始化
  pinMode(SERVO, OUTPUT);
  my_servo.attach(SERVO);
  my_servo.write(DOWN);
  delay(100); // 等待舵机转动
}

void loop() {
  // put your main code here, to run repeatedly:
  pidMotorControl();
  color = colorDetect();
  if (color != 0 && card_color == 0)
  {
    card_color = color; // 记录色卡颜色 
    showColor(card_color);
    expectedPWML -= 120;
    expectedPWMR -=120;
    color = 0;
    delay(500);
  }
  if (card_color != 0 && color == card_color)
  {
    motorRun(0,0);
    showColor(0);  // 显示‘N’
    expectedPWML += 120;
    expectedPWMR +=120;
    my_servo.write(UP);
    delay(450);
    my_servo.write(DOWN);
    delay(450);
  }
  delay(50);
}

// 检测到色卡
//void onceDetected()
//{
//  uint8_t count = 0;
//  pidMotorControl();
//  delay(100);
//  showColor(color);
//}

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

/*****************************
 * 8x8颜色显示模块
 ****************************/

//void turnRight(){
//  lc.setRow(0,0,B00010000);
//  lc.setRow(0,1,B11111111);
//  lc.setRow(0,2,B00100000);
//  lc.setRow(0,3,B00100000);
//  lc.setRow(0,4,B01111110);
//  lc.setRow(0,5,B11100010);
//  lc.setRow(0,6,B11100010);
//  lc.setRow(0,7,B01111110);
//}
//
//void turnLeft(){
//  lc.setRow(0,0,B00010000);
//  lc.setRow(0,1,B11111111);
//  lc.setRow(0,2,B00100000);
//  lc.setRow(0,3,B00100010);
//  lc.setRow(0,4,B01111110);
//  lc.setRow(0,5,B11001000);
//  lc.setRow(0,6,B10001000);
//  lc.setRow(0,7,B01111111);
//}
//
//void forward(){
//  lc.setRow(0,0,B00000000);
//  lc.setRow(0,1,B00111100);
//  lc.setRow(0,2,B00100000);
//  lc.setRow(0,3,B00111100);
//  lc.setRow(0,4,B00111100);
//  lc.setRow(0,5,B00100000);
//  lc.setRow(0,6,B00100000);
//  lc.setRow(0,7,B00000000);
//}
//
//void down(){
//  lc.setRow(0,0,B00000000);
//  lc.setRow(0,1,B00111100);
//  lc.setRow(0,2,B00100100);
//  lc.setRow(0,3,B00100100);
//  lc.setRow(0,4,B00100100);
//  lc.setRow(0,5,B00100100);
//  lc.setRow(0,6,B00111000);
//  lc.setRow(0,7,B00000000);
//}

void showColor(uint8_t color)
{
  if (color == 1)
  {
    // red
    lc.setRow(0, 0, B00000000);
    lc.setRow(0, 1, B00011100);
    lc.setRow(0, 2, B00010110);
    lc.setRow(0, 3, B00011100);
    lc.setRow(0, 4, B00011100);
    lc.setRow(0, 5, B00010100);
    lc.setRow(0, 6, B00010100);
    lc.setRow(0, 7, B00000000);
  }
  
  else if (color == 2)
  {
    // green
    lc.setRow(0, 0, B00001000);
    lc.setRow(0, 1, B00011100);
    lc.setRow(0, 2, B00010100);
    lc.setRow(0, 3, B00010000);
    lc.setRow(0, 4, B00011100);
    lc.setRow(0, 5, B00010000);
    lc.setRow(0, 6, B00011100);
    lc.setRow(0, 7, B00000000);
  }
  else if (color == 3)
  {
    // blue
    lc.setRow(0, 0, B00000000);
    lc.setRow(0, 1, B00011100);
    lc.setRow(0, 2, B00010100);
    lc.setRow(0, 3, B00011100);
    lc.setRow(0, 4, B00010110);
    lc.setRow(0, 5, B00010010);
    lc.setRow(0, 6, B00011100);
    lc.setRow(0, 7, B00000000);
  }
  else 
  {
    // no color detected
    lc.setRow(0, 0, B00000000);
    lc.setRow(0, 1, B00110100);
    lc.setRow(0, 2, B00110100);
    lc.setRow(0, 3, B00101100);
    lc.setRow(0, 4, B00101100);
    lc.setRow(0, 5, B00101100);
    lc.setRow(0, 6, B00100100);
    lc.setRow(0, 7, B00000000);
  }
}

/*******************************/

/*****************************
 * 颜色识别模块
 ****************************/
uint8_t colorDetect()
{
  tcs.getRGBC(&red, &green, &blue, &sum);
  tcs.lock();
  // 提取颜色值
  r = red; r /= sum;
  g = green; g /= sum;
  b = blue; b /= sum;
  // 颜色转换
  red = transform(r);
//  red = Filter.run(red);
  green = transform(g);
//  green = Filter.run(green);
  blue = transform(b);
//  blue = Filter.run(blue);
  Serial.println(green);
  if (red > green && red > blue && red > thresh_red)
  {
    return 1;
  }
  if (green > red && green > blue && green > thresh_green)
  {
    return 2;
  }
  if (blue > red && blue > green && blue > thresh_blue)
  {
    return 3;
  }
  else
    return 0;
}

uint32_t transform(float a)
{
  uint32_t ans = exp(10 * a) * 10;
  if (ans >= 10000)
    return 0;
  else
    return ans;
}
/***************************/

void pidMotorControl()
{
  gS1 = analogRead(grayScale1);
  gS2 = analogRead(grayScale2);
  gS1 = gS1>thresholdL?1:0;
  gS2 = gS2>thresholdR?1:0;
  //直行
    if(gS1==1&&gS2==1){
    motorRun(expectedPWML,expectedPWMR);
//    forward();
    }
  //左转
  if(gS1==0&&gS2==1){
    motorRun(expectedPWML-deltaPWM,expectedPWMR+deltaPWM);
//    turnLeft();
    }
  //右转
  if(gS1==1&&gS2==0){
    motorRun(expectedPWML+deltaPWM,expectedPWMR-deltaPWM);
//    turnRight();
    }
  //爬坡和爬楼梯
  if(gS1==0&&gS2==0){
    motorRun(expectedPWML,expectedPWMR);
//    down();
  }
}
