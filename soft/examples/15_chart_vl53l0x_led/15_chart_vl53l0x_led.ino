#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Wire.h>
#include <VL53L0X.h>

#if (HARDWARE == 120) 
  #define TFT_CS        0  // Не используется - в дисплее его нет!
  #define TFT_RST       10 // Дисплей не запускается без управления сброса RES
  #define TFT_DC        9  // Команды
#else
  #define TFT_CS        0  // Не используется - в дисплее его нет!
  #define TFT_RST       10 // Дисплей не запускается без управления сброса RES
  #define TFT_DC        9  // Команды
#endif
// Доп цвета
#define  ST77XX_DARKGREY  0x7BEF // Darker shade of grey 
#define ST77XX_LIGHTGREY  0xC618 // Lighter shade of grey

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
VL53L0X sensor;

#define SCREEN_W 240
#define SCREEN_H 240

#define LED_PIN 6

// Graph buffer
int graph[SCREEN_W];
int xPos = 1;

// Distance range (mm)
int minDist = 20;
int maxDist = 600;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

 // Инициализация дисплея
 #if (HARDWARE == 120)
  tft.init(240, 240, SPI_MODE3);// Новая версия
#else  
  tft.init(240, 240, SPI_MODE2);// Init ST7789 240x240 Параметр SPI_MODE2 ОБЯЗАТЕЛЕН! (без него дисплей не рабоатет!) для версии 110
#endif
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);  
  tft.cp437(true);

  // Init VL53L0X
   Wire.begin();
  sensor.setTimeout(500);
  if (!sensor.init())
  {
    Serial.println("Сенсор VL53L0X не найден!");
    while (1) {}
  }

#if defined LONG_RANGE
  // lower the return signal rate limit (default is 0.25 MCPS)
  sensor.setSignalRateLimit(0.1);
  // increase laser pulse periods (defaults are 14 and 10 PCLKs)
  sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
  sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
#endif

#if defined HIGH_SPEED
  // reduce timing budget to 20 ms (default is about 33 ms)
  sensor.setMeasurementTimingBudget(20000);
#elif defined HIGH_ACCURACY
  // increase timing budget to 200 ms
  sensor.setMeasurementTimingBudget(200000);
#endif
  // Clear graph buffer
  for (int i = 0; i < SCREEN_W; i++) graph[i] = SCREEN_H;
  drawGrid();
}

void loop() {
  int dist = readDistance();
  // LED logic
  if (dist > 0 && dist < 200) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }

  // Convert distance → Y position
  int y = map(dist, minDist, maxDist, SCREEN_H - 1, 0);
  y = constrain(y, 0, SCREEN_H - 1);

  // Clear current column
  tft.drawFastVLine(xPos, 0, SCREEN_H, ST77XX_BLACK);

  // Redraw grid line in this column (so grid persists)
  if (xPos % 40 == 0) {
    tft.drawFastVLine(xPos, 0, SCREEN_H, ST77XX_DARKGREY);
  }
  for (int gy = 0; gy < SCREEN_H; gy += 40) {
    tft.drawPixel(xPos, gy, ST77XX_DARKGREY);
  }

  // Draw line from previous point
  int prevX = (xPos - 1 + SCREEN_W) % SCREEN_W;
  int prevY = graph[prevX];

  tft.drawLine(prevX, prevY, xPos, y, ST77XX_GREEN);

  // Store value
  graph[xPos] = y;

  // Move forward
  xPos++;
  if (xPos >= SCREEN_W) xPos = 0;

  // Show distance text
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setTextSize(2);
  tft.print("Dist:");
  tft.print(dist);
  tft.print("mm  ");
  delay(60);
}

// ==== Distance Read ====
int readDistance() {
int distance;  
  distance=sensor.readRangeSingleMillimeters();
  if (sensor.timeoutOccurred()) { Serial.println(" TIMEOUT");return maxDist;  }
  else return distance; 
}

// ==== Grid ====
void drawGrid() {
  // Horizontal lines
  for (int y = 0; y < SCREEN_H; y += 40) {
    tft.drawFastHLine(0, y, SCREEN_W, ST77XX_DARKGREY);
  }
  // Vertical lines
  for (int x = 0; x < SCREEN_W; x += 40) {
    tft.drawFastVLine(x, 0, SCREEN_H, ST77XX_DARKGREY);
  }
}
