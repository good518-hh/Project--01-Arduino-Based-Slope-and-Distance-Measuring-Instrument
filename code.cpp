#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>
//未安装这些库，则需要在工具栏中库管理工具中下载
// ------------------ OLED 参数 ------------------
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ------------------ 引脚定义 ------------------
const byte TRIG1 = 2;   // 超声波 1 Trig
const byte ECHO1 = 3;   // 超声波 1 Echo
const byte TRIG2 = 4;   // 超声波 2 Trig
const byte ECHO2 = 5;   // 超声波 2 Echo
const byte TILT  = A0;  // 倾斜传感器（开关量，低电平＝水平）
const byte LED   = 8;   // 外接 LED 正极（负极串 220 Ω 到 GND）

// ------------------ 全局变量 ------------------
float d1 = 0;   // 超声波 1 距离（cm）
float d2 = 0;   // 超声波 2 距离（cm）

// =========================================================
// 函数：getDist
// 功能：向指定超声波模块发 10 µs 脉冲并读取回波时间
// 参数：trig 触发脚号，echo 回响脚号
// 返回：距离（cm），保留两位小数；超时或无回波返回 -1
// =========================================================
float getDist(byte trig, byte echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);           // 确保低电平
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);          // 触发脉冲 ≥10 µs
  digitalWrite(trig, LOW);

  // 30 ms 超时，避免程序卡死
  unsigned long t = pulseIn(echo, HIGH, 30000);

  // t==0 表示超时，返回 -1；否则用 t/58.0 得厘米
  return (t > 0) ? (t / 58.0) : -1;
}

// =========================================================
// 函数：setup
// 功能：初始化串口、引脚、OLED
// =========================================================
void setup() {
  Serial.begin(9600);

  // 设置引脚方向
  pinMode(TRIG1, OUTPUT); pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT); pinMode(ECHO2, INPUT);
  pinMode(TILT, INPUT_PULLUP);  // 内部上拉，低电平触发
  pinMode(LED, OUTPUT);

  // OLED 初始化
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 初始化失败"));
    while (true) delay(10);
  }
  display.clearDisplay();
  display.display();
}

// =========================================================
// 函数：loop
// 功能：循环读取双超声波距离，计算夹角，OLED 实时显示
// =========================================================
void loop() {
  // 1. 读取两路距离
  float tmp1 = getDist(TRIG1, ECHO1);
  float tmp2 = getDist(TRIG2, ECHO2);

  // 2. 若读数有效，则更新全局变量（简单滤波）
  if (tmp1 >= 0) d1 = tmp1;
  if (tmp2 >= 0) d2 = tmp2;

  // 3. 计算夹角（示例公式：atan(2/(d1-d2))）
  //    注意：d1-d2 可能为 0，需保护
  float deg = 0;
  float delta = d1 - d2;
  if (abs(delta) > 0.01) {        // 避免零除
    deg = atan(2.0 / delta) * 57.2957795131; // rad → deg
  }

  // 4. 读取倾斜开关
  bool level = (digitalRead(TILT) == LOW);  // 低电平 = 水平
  digitalWrite(LED, level ? HIGH : LOW);    // LED 指示

  // 5. OLED 显示
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.print("S1:"); display.print(d1, 2); display.println(" cm");
  display.print("S2:"); display.print(d2, 2); display.println(" cm");
  display.println(level ? "LEVEL" : "TILTED");
  display.print("deg:"); display.print(deg, 2);

  display.display();

  // 6. 刷新周期
  delay(200);   // 每 200 ms 更新一次
}