/********************
 * 2022.6.3 19:10 编辑
 * 除去了两轮差速
 * 传感器被抬高
 * 改变舵机端口
 */
#include <Servo.h>
#include "LedControl.h"
#include <Wire.h>        //调用IIC库函数
#include "MH_TCS34725.h" //调用颜色识别传感器库函数
#ifdef __AVR__
#include <avr/power.h>
#endif
#include <MsTimer2.h>
#define grayScale1 A2
#define grayScale2 A3
#define motor1L 5
#define motor1R 6
#define motor2L 9
#define motor2R 10
#define SERVO 11
#define encoderTime 1000

// 常量
uint8_t SPEED = 220;
uint8_t theta = 20; // 左右轮速度差
uint8_t deltaPWM = 210;
uint8_t de_speed = 180; // 识别色卡后减速幅度

//初始化目标速度
uint8_t expectedPWML = SPEED - theta;
uint8_t expectedPWMR = SPEED;

//采集到黑线时，传感器为低电平
uint8_t gS1 = 1;
uint8_t gS2 = 1;
uint8_t thresholdR = 120; // 120->110
uint8_t thresholdL = 120;
LedControl lc = LedControl(12, 11, 13, 1);

// 颜色识别相关
// 颜色阈值
uint16_t thresh_red = 1500;
uint16_t thresh_green = 1400;
uint16_t thresh_blue = 1900;
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

// 时间定时器
uint8_t time_counter = 0;
uint8_t time_delay = 25;
const uint8_t total_delay = 40; // 总延时时间 40s
uint16_t sensor_delay = 800;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(grayScale1, INPUT);
  pinMode(grayScale2, INPUT);
  pinMode(motor1L, OUTPUT);
  pinMode(motor1R, OUTPUT);
  pinMode(motor2L, OUTPUT);
  pinMode(motor2R, OUTPUT);
  MsTimer2::set(encoderTime, onTimer);
  MsTimer2::start();
  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);
  if (!tcs.begin())
  {
    while (1)
      ;
  }
  // 舵机初始化
  pinMode(SERVO, OUTPUT);
}

void loop()
{
  // put your main code here, to run repeatedly:
  // pidMotorControl();
  /********************
   *  传感器调试程序
   */
    Serial.print(analogRead(grayScale1));
    Serial.print(",");
    Serial.println(analogRead(grayScale2));
  /*******************/
  // delay(time_delay);
  // if (time_counter == total_delay)
  // {
  //   expectedPWML -= 50;
  //   expectedPWMR -= 50;
  //   hitBall();
  // }
  // hitBall();
}

/*****************************
 *  定时器中断函数
 * ****************************/
void onTimer()
{
  if (time_counter < total_delay)
    ++time_counter;
}

/******************************
 *  扎气球程序
 * ***************************/
void hitBall()
{
  color = colorDetect();
  if (color != 0 && card_color == 0)
  {
    card_color = color; // 记录色卡颜色
    showColor(card_color);
    expectedPWML -= de_speed;
    expectedPWMR -= de_speed;
    deltaPWM -= de_speed;
    color = 0;
    motorRun(0, 0); // 开始减速
    delay(200);
  }
  if (card_color != 0 && color == card_color)
  {
    motorRun(0, 0);
    down(); //显示‘D’
    my_servo.attach(SERVO);
    my_servo.write(UP);
    delay(sensor_delay);
    my_servo.write(DOWN);
    delay(sensor_delay);
    my_servo.detach();
    expectedPWML += de_speed;
    expectedPWMR += de_speed;
    deltaPWM += de_speed;
  }
}

/*****************************
 * 马达驱动
 * ***************************/
void motorRun(int left, int right)
{
  left = left > 255 ? 255 : left;
  left = left <= 0 ? 0 : left;
  right = right > 255 ? 255 : right;
  right = right <= 0 ? 0 : right;
  analogWrite(motor1L, left);
  analogWrite(motor2R, right);
}

/*****************************
 * 8x8颜色显示模块
 ****************************/

void turnRight()
{
  lc.setRow(0, 0, B00010000);
  lc.setRow(0, 1, B11111111);
  lc.setRow(0, 2, B00100000);
  lc.setRow(0, 3, B00100000);
  lc.setRow(0, 4, B01111110);
  lc.setRow(0, 5, B11100010);
  lc.setRow(0, 6, B11100010);
  lc.setRow(0, 7, B01111110);
}

void turnLeft()
{
  lc.setRow(0, 0, B00010000);
  lc.setRow(0, 1, B11111111);
  lc.setRow(0, 2, B00100000);
  lc.setRow(0, 3, B00100010);
  lc.setRow(0, 4, B01111110);
  lc.setRow(0, 5, B11001000);
  lc.setRow(0, 6, B10001000);
  lc.setRow(0, 7, B01111111);
}

void forward()
{
  lc.setRow(0, 0, B00000000);
  lc.setRow(0, 1, B00111100);
  lc.setRow(0, 2, B00100000);
  lc.setRow(0, 3, B00111100);
  lc.setRow(0, 4, B00111100);
  lc.setRow(0, 5, B00100000);
  lc.setRow(0, 6, B00100000);
  lc.setRow(0, 7, B00000000);
}

void down()
{
  lc.setRow(0, 0, B00000000);
  lc.setRow(0, 1, B00111100);
  lc.setRow(0, 2, B00100100);
  lc.setRow(0, 3, B00100100);
  lc.setRow(0, 4, B00100100);
  lc.setRow(0, 5, B00100100);
  lc.setRow(0, 6, B00111000);
  lc.setRow(0, 7, B00000000);
}

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
  r = red;
  r /= sum;
  g = green;
  g /= sum;
  b = blue;
  b /= sum;
  // 颜色转换
  red = transform(r);
  green = transform(g);
  blue = transform(b);
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

uint16_t transform(float a)
{
  uint16_t ans = exp(10 * a) * 10;
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
  gS1 = gS1 > thresholdL ? 1 : 0;
  gS2 = gS2 > thresholdR ? 1 : 0;
  //直行
  if (gS1 == 1 && gS2 == 1)
  {
    motorRun(expectedPWML, expectedPWMR);
    forward();
  }
  //左转
  if (gS1 == 0 && gS2 == 1)
  {
    motorRun(expectedPWML - deltaPWM, expectedPWMR + deltaPWM);
    turnLeft();
  }
  //右转
  if (gS1 == 1 && gS2 == 0)
  {
    motorRun(expectedPWML + deltaPWM, expectedPWMR - deltaPWM);
    turnRight();
  }
  //爬坡和爬楼梯
  if (gS1 == 0 && gS2 == 0)
  {
    motorRun(expectedPWML - 10, expectedPWMR); // 爬坡时左转
    down();
  }
}
