#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_VL53L0X.h>

#if (HARDWARE == 120) 
  #define TFT_CS        0  // Не используется - в дисплее его нет!
  #define TFT_RST       10 // Дисплей не запускается без управления сброса RES
  #define TFT_DC        9  // Команды
#else
  #define TFT_CS        0  // Не используется - в дисплее его нет!
  #define TFT_RST       10 // Дисплей не запускается без управления сброса RES
  #define TFT_DC        9  // Команды
#endif  


Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

// Область графики
#define GRAPH_X  20
#define GRAPH_Y  40
#define GRAPH_W  220
#define GRAPH_H  160

#define MIN_DIST 0
#define MAX_DIST 600

int currentX = GRAPH_X;

void drawAxes() {
  tft.drawRect(GRAPH_X, GRAPH_Y, GRAPH_W, GRAPH_H, ST77XX_WHITE);

  // Вертикальная ось
  for (int d = 0; d <= MAX_DIST; d += 100) {
    int y = GRAPH_Y + GRAPH_H - (long)d * GRAPH_H / MAX_DIST;

    tft.drawLine(GRAPH_X - 4, y, GRAPH_X, y, ST77XX_WHITE);

    tft.setCursor(0, y - 5);
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.print(d);
  }
}

int mapDistanceToY(int distance) {
  if (distance < MIN_DIST) distance = MIN_DIST;
  if (distance > MAX_DIST) distance = MAX_DIST;
  return GRAPH_Y + GRAPH_H - 
         (long)(distance - MIN_DIST) * GRAPH_H / (MAX_DIST - MIN_DIST);
}

void setup() {
  Serial.begin(115200);
  // Инициализация дисплея
 #if (HARDWARE == 120)
  tft.init(240, 240, SPI_MODE3);           // Новая версия
#else  
  tft.init(240, 240, SPI_MODE2);           // Init ST7789 240x240 Параметр SPI_MODE2 ОБЯЗАТЕЛЕН! (без него дисплей не рабоатет!) для версии 110
#endif
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);  
  tft.cp437(true);

  if (!lox.begin()) { // Инициализация датчика
    tft.setCursor(20, 20);
    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(2);
    tft.print("VL53L0X FAIL");
    while (1);
  }

  drawAxes();
}

void loop() {
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);

  if (measure.RangeStatus != 4) { // Если не зашкал
    int distance = measure.RangeMilliMeter;

    int y = mapDistanceToY(distance);
    // Draw pixel (chart point)
    tft.drawPixel(currentX, y, ST77XX_GREEN);
    // Clear previous value area
    tft.fillRect(60, 10, 120, 20, ST77XX_BLACK);

    // Показать дистанцию
    tft.setCursor(60, 10);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setTextSize(2);
    tft.print(distance);
    tft.print(" mm");

    currentX++;

    // Если дошли до конца -> стираем и заново
    if (currentX >= GRAPH_X + GRAPH_W) {
      currentX = GRAPH_X;

      tft.fillRect(GRAPH_X + 1, GRAPH_Y + 1, 
                   GRAPH_W - 2, GRAPH_H - 2, ST77XX_BLACK);

      drawAxes();
    }
  }
  delay(50);
}
