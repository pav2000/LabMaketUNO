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

// Scale settings
#define SCALE_X  00
#define SCALE_Y  150
#define SCALE_LEN 240
#define MIN_DIST 0
#define MAX_DIST 800  // mm
int lastBarX = -1;

void setup()
{
  Serial.begin(115200);
  Serial.println(F("Скетч использует 19776 байт (61%) памяти устройства"));
  Serial.println(F("Глобальные переменные используют 641 байт (22%) динамической памяти, оставляя 1407 байт"));
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
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.setCursor(0,10);
  tft.println(utf8rus(" Датчик VL53L0X"));
  tft.setCursor(0,45);
  tft.setTextSize(3);
  tft.setTextColor(ST77XX_BLUE);
  tft.println(utf8rus("Дист."));
  drawScale();
  
  Wire.begin();
  sensor.setTimeout(500);
  if (!sensor.init())
  {
    Serial.println("Failed to detect and initialize sensor!");
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
int l;  
  Serial.print(F("Reading a measurement... "));
  l=sensor.readRangeSingleMillimeters();
  Serial.println(l);
  drawDistanceBar(l);
  if (sensor.timeoutOccurred()) { Serial.println(" TIMEOUT"); }
  delay(200);
}


// Начертить шкалу
void drawScale() {
  tft.drawLine(SCALE_X, SCALE_Y, SCALE_X + SCALE_LEN, SCALE_Y, ST77XX_WHITE);

  // Начертить отсчеты 100 mm
  for (int d = MIN_DIST; d <= MAX_DIST; d += 100) {
    int x = SCALE_X + (long)(d - MIN_DIST) * SCALE_LEN / (MAX_DIST - MIN_DIST);

    // Основые тики
    tft.drawLine(x, SCALE_Y - 8, x, SCALE_Y + 8, ST77XX_WHITE);

    // Значения
    if(d<MAX_DIST-1){
      tft.setCursor(x - 10, SCALE_Y + 12);
      tft.setTextSize(1);
      tft.setTextColor(ST77XX_WHITE);
      tft.print(d);
      }
  }
}

// Нарисовать новое значение 
void drawDistanceBar(int distance) {
  // Clamp value
  if (distance < MIN_DIST) distance = MIN_DIST;
  if (distance > MAX_DIST) distance = MAX_DIST;

  int barX = SCALE_X + (long)(distance - MIN_DIST) * SCALE_LEN / (MAX_DIST - MIN_DIST);

  // Стереть старую шкалу
  if (lastBarX >= 0) {
   tft.fillRect(lastBarX, SCALE_Y - 20,240- lastBarX, 10, ST77XX_BLACK);
  }
  // Начертить новую шкалу
  tft.fillRect(0, SCALE_Y - 20, lastBarX, 10, ST77XX_RED);
  lastBarX = barX;

  // Вывести значение
  tft.fillRect(80, 40, 160, 30, ST77XX_BLACK);
  tft.setCursor(120, 45);
  tft.setTextSize(3);
  tft.setTextColor(ST77XX_GREEN);
  tft.print(distance);
  tft.print(" mm");  
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
