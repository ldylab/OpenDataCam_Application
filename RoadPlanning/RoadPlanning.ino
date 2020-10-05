#include <LedControl.h>
#include <FastLED.h>

//WS2812的定义
#define LED_PIN     7
#define NUM_LEDS    46
CRGB leds[NUM_LEDS];

//每一列灯的个数
#define Line_0_num  16
#define Line_1_num  15
#define Line_2_num  15

//点阵的定义
#define DIN         11
#define CS          10
#define CLK         13
#define MatrixNUM   4

//定义信号传输的PIN
#define Signal_1    A0
#define Signal_2    A1
#define Signal_3    A2

LedControl lc = LedControl(DIN, CLK, CS, MatrixNUM);

void setup()
{
  //WS2812初始化
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);

  //点阵初始化
  int devices = lc.getDeviceCount();
  for (int address = 0; address < devices; address++)
  {
    lc.shutdown(address, false);
    //设定每一个LED模块的亮度
    lc.setIntensity(address, 8);
    //LED显示清理
    lc.clearDisplay(address);
  }

  //JetsonNano通讯初始化
  pinMode(Signal_1, INPUT);
  pinMode(Signal_2, INPUT);
  pinMode(Signal_3, INPUT);

  //串口初始化
  Serial.begin(9600);
}

void loop()
{
  int talk = Communicate_Jetson();
  Serial.println(talk);
  Change_Dynamics(talk);
}

//四种过渡模式的自动切换
void Change_Dynamics(int mode)
{
  static int mode_last = -1;

  if(mode != mode_last)
  {
    if(mode == 0)
    {
      //变化路口的显示
      Display_Arrow(0, 0);
      Display_Arrow(2, 1);
      Display_Arrow(1, 2);
      Display_Arrow(1, 3);
      Display_Change(0);
      Display_Mode(1);
    }
    if(mode == 1)
    {
      //变化路口的显示
      Display_Arrow(0, 0);
      Display_Arrow(3, 1);
      Display_Arrow(1, 2);
      Display_Arrow(1, 3);
      Display_Change(1);
      Display_Mode(0);
    }
    if(mode == 2)
    {
      //变化路口的显示
      Display_Arrow(0, 0);
      Display_Arrow(0, 1);
      Display_Arrow(2, 2);
      Display_Arrow(1, 3);
      Display_Change(2);
      Display_Mode(2);
    }
    if(mode == 3)
    {
      //变化路口的显示
      Display_Arrow(0, 0);
      Display_Arrow(0, 1);
      Display_Arrow(3, 2);
      Display_Arrow(1, 3);
      Display_Change(3);
      Display_Mode(1);
    }
  }
  mode_last = mode;
}

//四种过度模式
//输入：输入0-3，变换四种转化的模式
//输出：LED直接输出闪烁的亮度变化
void Display_Change(int mode)
{
  float R = 255;
  float G = 128;
  float B = 34;
  float R_tmp = R;
  float G_tmp = G;
  float B_tmp = B;

  int value_speed = 20; //LED亮度渐变的柔和程度，数值越大越柔和
  int change_speed = 300; //闪烁的毫秒程度

  int line_start, line_num, line_mode, line_next;

  switch (mode)
  {
    case 0:
      //起始点亮的灯
      line_start = 0;
      line_num = Line_0_num;
      line_next = Line_1_num;
      line_mode = 0; //就是反过来的意思
      break;
    case 1:
      //起始点亮的灯
      line_start = 0;
      line_num = Line_0_num;
      line_next = Line_1_num;
      line_mode = 1; //就是反过来的意思
      break;
    case 2:
      //起始点亮的灯
      line_start = Line_0_num;
      line_num = Line_1_num;
      line_next = Line_2_num;
      line_mode = 0; //就是反过来的意思
      break;
    case 3:
      //起始点亮的灯
      line_start = Line_0_num;
      line_num = Line_1_num;
      line_next = Line_2_num;
      line_mode = 1; //就是反过来的意思
      break;
    default:
      break;
  }

  if (line_mode == 0)
  {
    //逐渐熄灭测试
    for (int j = 0; j < value_speed; j++)
    {
      R = R - R_tmp / value_speed;
      G = G - G_tmp / value_speed;
      B = B - B_tmp / value_speed;
      //逐一的点亮灯
      for (int i = line_start; i < line_start + line_num; i++)
      {
        leds[i] = CRGB(R, G, B);
        leds[i + line_next] = CRGB(R_tmp - R, G_tmp - G, B_tmp - B);
      }
      FastLED.show();
      delay(change_speed);
      for (int i = line_start; i < line_start + line_num; i++)
      {
        leds[i] = CRGB(0, 0, 0);
        leds[i + line_next] = CRGB(0, 0, 0); //传入的是下个LED的数量
      }
      FastLED.show();
      delay(change_speed);
    }
  }
  else if (line_mode == 1)
  {
    R = 0;
    G = 0;
    B = 0;
    //逐渐点亮测试
    for (int j = 0; j < value_speed; j++)
    {
      R = R + R_tmp / value_speed;
      G = G + G_tmp / value_speed;
      B = B + B_tmp / value_speed;
      for (int i = line_start; i < line_start + line_num; i++)
      {
        leds[i] = CRGB(R, G, B);
        leds[i + line_next] = CRGB(R_tmp - R, G_tmp - G, B_tmp - B);
      }
      FastLED.show();
      delay(change_speed);
      for (int i = line_start; i < line_start + line_num; i++)
      {
        leds[i] = CRGB(0, 0, 0);
        leds[i + line_next] = CRGB(0, 0, 0);
      }
      FastLED.show();
      delay(change_speed);
    }
  }
}

//LED和MatrixLED平时的显示
//输入要显示的模式，三种模式
void Display_Mode(int mode)
{
  switch (mode)
  {
    case 0:
      Display_Arrow(0, 0);
      Display_Arrow(1, 1);
      Display_Arrow(1, 2);
      Display_Arrow(1, 3);
      Display_Line(0, 1);
      Display_Line(1, 0);
      Display_Line(2, 0);
      break;
    case 1:
      Display_Arrow(0, 0);
      Display_Arrow(0, 1);
      Display_Arrow(1, 2);
      Display_Arrow(1, 3);
      Display_Line(0, 0);
      Display_Line(1, 1);
      Display_Line(2, 0);
      break;
    case 2:
      Display_Arrow(0, 0);
      Display_Arrow(0, 1);
      Display_Arrow(0, 2);
      Display_Arrow(1, 3);
      Display_Line(0, 0);
      Display_Line(1, 0);
      Display_Line(2, 1);
      break;
    default:
      break;
  }
}

//与JetsonNano通讯
//输入：无，直接调用即可
//输出：返回Jetson要求的哪一种模式
int Communicate_Jetson()
{
  int now_mode;
  static int last_mode = 1;
  
  if (digitalRead(Signal_1) == HIGH && digitalRead(Signal_2) == LOW && digitalRead(Signal_3) == LOW)
  {
    now_mode = 0;
  }
  else if (digitalRead(Signal_1) == LOW && digitalRead(Signal_2) == HIGH && digitalRead(Signal_3) == LOW)
  {
    now_mode = 1;
  }
  else if (digitalRead(Signal_1) == LOW && digitalRead(Signal_2) == LOW && digitalRead(Signal_3) == HIGH)
  {
    now_mode = 2;
  }

  if(last_mode == 1 && now_mode == 0)
  {
    last_mode = now_mode;
    return 1;
  }
  else if(last_mode == 0 && now_mode == 1)
  {
    last_mode = now_mode;
    return 0;
  }
  else if(last_mode == 2 && now_mode == 1)
  {
    last_mode = now_mode;
    return 3;
  }
  else if(last_mode == 1 && now_mode == 2)
  {
    last_mode = now_mode;
    return 2;
  }
}

//显示道路分界线中的LED
//输入：1）哪一个道路旁边的LED要亮起来，2）输入要LED亮（1）还是灭（0）
//输出：2）哪一个道路的LED亮起
void Display_Line(int which_line, int line_status)
{
  //显示的颜色
  int R = 255;
  int G = 128;
  int B = 34;

  switch (which_line)
  {
    case 0:
      for (int i = 0; i < Line_0_num; i++)
      {
        if (line_status == 1)
        {
          leds[i] = CRGB(R, G, B);
        }
        else
        {
          leds[i] = CRGB(0, 0, 0);
        }
      }
      FastLED.show();
      break;
    case 1:
      for (int i = Line_0_num; i < Line_0_num + Line_1_num; i++)
      {
        if (line_status == 1)
        {
          leds[i] = CRGB(R, G, B);
        }
        else
        {
          leds[i] = CRGB(0, 0, 0);
        }
      }
      FastLED.show();
      break;
    case 2:
      for (int i = Line_0_num + Line_1_num; i < Line_0_num + Line_1_num + Line_2_num; i++)
      {
        if (line_status == 1)
        {
          leds[i] = CRGB(R, G, B);
        }
        else
        {
          leds[i] = CRGB(0, 0, 0);
        }
      }
      FastLED.show();
      break;
    default:
      break;
  }
}

//点阵上显示四种箭头，包含异常处理
//输入：1）显示箭头的种类，2）哪一个点阵显示
//输出：显示于点阵上
byte Display_Arrow(int which_arrow, int display_device)
{
  byte arrow[5][8] = {{0x18, 0x3C, 0x7E, 0xFF, 0x18, 0x18, 0x18, 0x18}, //0--向上
    {0x18, 0x18, 0x18, 0x18, 0xFF, 0x7E, 0x3C, 0x18}, //1--向下
    {0x10, 0x30, 0x70, 0xFF, 0xFF, 0x70, 0x30, 0x10}, //2--向左
    {0x8, 0xC, 0xE, 0xFF, 0xFF, 0xE, 0xC, 0x8 }, //3--向右
    {0xE7, 0x81, 0x81, 0x18, 0x18, 0x81, 0x81, 0xE7}
  };//other--异常显示

  if (display_device >= MatrixNUM)
  {
    for (int i = 0; i < MatrixNUM; i++)
    {
      printByte(arrow[4], i);
    }
    return 0;
  }

  switch (which_arrow)
  {
    case 0:
      printByte(arrow[0], display_device);
      break;
    case 1:
      printByte(arrow[1], display_device);
      break;
    case 2:
      printByte(arrow[2], display_device);
      break;
    case 3:
      printByte(arrow[3], display_device);
      break;
    default:
      printByte(arrow[4], display_device);
      break;
  }
}

//点阵显示
//输入：1）显示图像的数组（使用8*8取模软件取出即可），2）点阵地址哪一个点阵显示
//输出：点阵显示所输入的数组
void printByte(byte character [], int address)
{
  int i = 0;
  for (i = 0; i < 8; i++)
  {
    lc.setRow(address, i, character[i]);
  }
}
