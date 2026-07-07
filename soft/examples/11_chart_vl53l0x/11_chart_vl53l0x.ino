/* The range readings are in units of mm. */
 
// https://github.com/pololu/vl53l0x-arduino
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <Wire.h>
#include <VL53L0X.h>

// Uncomment this line to use long range mode. This
// increases the sensitivity of the sensor and extends its
// potential range, but increases the likelihood of getting
// an inaccurate reading because of reflections from objects
// other than the intended target. It works best in dark
// conditions.
//#define LONG_RANGE


// Uncomment ONE of these two lines to get
// - higher speed at the cost of lower accuracy OR
// - higher accuracy at the cost of lower speed

//#define HIGH_SPEED
//#define HIGH_ACCURACY

#define HARDWARE  120  // Номер версии платы (есть еще 110 версия)
#define VERSION   0.8  // Номер версии ПО

  // For the breakout board, you can use any 2 or 3 pins.
  // These pins will also work for the 1.8" TFT shield.
#if (HARDWARE == 120) 
  #define TFT_CS        0  // Не используется - в дисплее его нет!
  #define TFT_RST       10 // Дисплей не запускается без управления сброса RES
  #define TFT_DC        9  // Команды
#else
  #define TFT_CS        0  // Не используется - в дисплее его нет!
  #define TFT_RST       10 // Дисплей не запускается без управления сброса RES
  #define TFT_DC        9  // Команды
#endif  

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);  // Объявление дисплея
VL53L0X sensor;
// Chart settings
#define GRAPH_X  20
#define GRAPH_Y  40
#define GRAPH_W  280
#define GRAPH_H  160

#define MIN_DIST 0
#define MAX_DIST 800

int currentX = GRAPH_X;

// Отобразить оси
void drawAxes() {
  tft.drawRect(GRAPH_X, GRAPH_Y, GRAPH_W, GRAPH_H, ST77XX_WHITE);

  // Вертикальная ось
  for (int d = 0; d <= MAX_DIST; d += 100) {
    int y = GRAPH_Y + GRAPH_H - (long)d * GRAPH_H / MAX_DIST;
    tft.drawLine(GRAPH_X - 5, y, GRAPH_X, y, ST77XX_WHITE);
    tft.setCursor(0, y - 5);
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.print(d);
  }
}

// Перевод расстояния в точки
int mapDistanceToY(int distance) {
  if (distance < MIN_DIST) distance = MIN_DIST;
  if (distance > MAX_DIST) distance = MAX_DIST;
  return GRAPH_Y + GRAPH_H - (long)(distance - MIN_DIST) * GRAPH_H / (MAX_DIST - MIN_DIST);
}

void setup()
{
  Serial.begin(115200);
  Serial.println(F("Скетч использует 17616 байт (54%) памяти устройства"));
  Serial.println(F("Глобальные переменные используют 598 байт (29%) динамической памяти, оставляя 1450 байт"));
  Serial.println(F("Popolu VL53L0X test TFT screen"));
  
 // Инициалаизация дисплея
 #if (HARDWARE == 120)
  tft.init(240, 240, SPI_MODE3);           // Новая версия
#else  
  tft.init(240, 240, SPI_MODE2);           // Init ST7789 240x240 Параметр SPI_MODE2 ОБЯЗАТЕЛЕН! (без него дисплей не рабоатет!) для версии 110
#endif
  tft.setRotation(1);
  tft.cp437(true);
 
  Serial.println(F("Initialized ST7789"));
  tft.fillScreen(ST77XX_BLACK);
  drawAxes();
  
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
}

void loop()
{
int distance;  
  distance=sensor.readRangeSingleMillimeters();
  if (sensor.timeoutOccurred()) { Serial.println(" TIMEOUT"); }
  else {
    int y = mapDistanceToY(distance);
    // Отобразить точку
    tft.drawPixel(currentX, y, ST77XX_GREEN);
    // Отобразить значение
    tft.fillRect(60, 10, 200, 20, ST77XX_BLACK);
    tft.setCursor(60, 10);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setTextSize(2);
    tft.print(distance);
    tft.print(" mm");
    currentX++;
    // Скролл
    if (currentX >= GRAPH_X + GRAPH_W) {
      currentX = GRAPH_X;
      // Очистить 
      tft.fillRect(GRAPH_X + 1, GRAPH_Y + 1, GRAPH_W - 2, GRAPH_H - 2, ST77XX_BLACK);
      drawAxes();
    }
  }
  delay(100);
}

// Перекодировка на лету  UTF-8 в Windows-1251 
String utf8rus(String source)
{
  int i,k;
  String target;
  unsigned char n;
  char m[2] = { '0', '\0' };

  k = source.length(); i = 0;

  while (i < k) {
    n = source[i]; i++;

    if (n >= 0xC0) {
      switch (n) {
        case 0xD0: {
          n = source[i]; i++;
          if (n == 0x81) { n = 0xA8; break; }
          if (n >= 0x90 && n <= 0xBF) n = n + 0x30;
          break;
        }
        case 0xD1: {
          n = source[i]; i++;
          if (n == 0x91) { n = 0xB8; break; }
          if (n >= 0x80 && n <= 0x8F) n = n + 0x70;
          break;
        }
      }
    }
    m[0] = n; target = target + String(m);
  }
return target;
}
