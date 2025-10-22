#include <Adafruit_NeoPixel.h>

// ======== NeoPixel 设置 ========
#define NEOPIXEL_PIN 6      // LED 数据线连接引脚
#define NUMPIXELS 6         // 本地测试使用的 LED 数量
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// ======== HC-SR04 超声波传感器 ========
#define TRIG_PIN 8
#define ECHO_PIN 9

// ======== 全局变量 ========
long lastDistance = 0;           // 上一次测量的距离
int currentR = 0, currentG = 0, currentB = 255;  // 初始颜色蓝色
int baseBrightness = 2;        // 初始亮度，灯条默认亮度
int maxBrightness = 5;         // 最大亮度，距离变化大时使用

// ======== 函数声明 ========
long readDistanceCM();                                   // 测距函数
void adaptiveFlashColorLocal(int newR, int newG, int newB, int delta); // 自适应速度+亮度
void setOverallBrightness(int brightness);              // 设置整体亮度

// ======== Setup 初始化 ========
void setup() {
  Serial.begin(115200);

  pixels.begin();                     // 初始化 LED
  setOverallBrightness(baseBrightness); // 设置初始亮度
  pixels.show();                      // 显示初始状态

  pinMode(TRIG_PIN, OUTPUT);          // 超声波发射引脚
  pinMode(ECHO_PIN, INPUT);           // 超声波接收引脚

  Serial.println("✅ 本地测试初始化完成");
}

// ======== Main Loop 主循环 ========
void loop() {
  long distance = readDistanceCM();   // 读取距离
  if (distance <= 0) return;          // 无效读数直接跳过

  // 限制距离在 5~100 cm 内，超出范围保持蓝色
  bool outOfRange = false;
  if (distance < 5 || distance > 80) {
    distance = 100;  // 默认蓝色对应最远
    outOfRange = true;
  }

  int newR, newG, newB;

  if (outOfRange) {
    // 超出范围保持蓝色
    newR = 0;
    newG = 0;
    newB = 255;
  } else {
    // 近→红色，远→蓝色
    if (distance < 20) {          // 非常近
      newR = 255; 
      newG = map(distance, 5, 20, 0, 100); // 逐渐增加绿色
      newB = 0;
    } else if (distance < 50) {   // 近
      newR = map(distance, 20, 40, 255, 0); // 红色逐渐减小
      newG = map(distance, 20, 40, 100, 0); // 绿色逐渐减小
      newB = map(distance, 20, 40, 0, 100); // 蓝色逐渐增加
    } else if (distance < 80) {   // 远
      newR = 0;
      newG = map(distance, 40, 60, 0, 50); // 增加一点绿色
      newB = map(distance, 40, 60, 100, 200); // 蓝色增加
    } else {                       // 很远
      newR = 0;
      newG = 0;
      newB = 255;                   // 最远蓝色
    }
  }

  int delta = abs(distance - lastDistance); // 距离变化量

  if (delta > 1) {
    // 距离变化明显，调用自适应函数
    adaptiveFlashColorLocal(newR, newG, newB, delta);
    currentR = newR; 
    currentG = newG; 
    currentB = newB;
  } else {
    // 距离变化很小，保持初始亮度，不提高亮度
    pixels.setBrightness(baseBrightness);
    // 直接更新颜色，但亮度保持 baseBrightness
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(newR, newG, newB));
    }
    pixels.show();
  }

  lastDistance = distance;
  delay(100); // 延时 100ms 循环
}

// ======== HC-SR04 测距函数 ========
long readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 最多30ms
  if (duration == 0) {
    Serial.println("⚠️ 无效距离");
    return -1;
  }

  long distance = duration / 58.2; // 转换为厘米
  Serial.print("📏 当前距离: ");
  Serial.print(distance);
  Serial.println(" cm");
  return distance;
}

// ======== 自适应速度和亮度变化 ========
void adaptiveFlashColorLocal(int newR, int newG, int newB, int delta) {
  // 根据距离变化调整 LED 扫动速度
  int stepDelay;
  if (delta < 3) stepDelay = 120;   // 小变化慢
  else if (delta < 15) stepDelay = 60; // 中等
  else stepDelay = 15;               // 大变化快

  // 根据 delta 调整亮度：变化大→亮，变化小→暗
  int dynamicBrightness = map(delta, 1, 30, baseBrightness, maxBrightness);
  dynamicBrightness = constrain(dynamicBrightness, baseBrightness, maxBrightness);
  pixels.setBrightness(dynamicBrightness);

  Serial.print("Δ=");
  Serial.print(delta);
  Serial.print("  速度=");
  Serial.print(stepDelay);
  Serial.print("ms  亮度=");
  Serial.println(dynamicBrightness);

  // 从下到上依次变色
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(newR, newG, newB));
    pixels.show();
    delay(stepDelay);
  }
}

// ======== 设置整体亮度 ========
void setOverallBrightness(int brightness) {
  // brightness: 0~255
  pixels.setBrightness(brightness);
  Serial.print("💡 整体亮度设置为: ");
  Serial.println(brightness);
}
