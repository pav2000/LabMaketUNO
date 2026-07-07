#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <DFRobot_QMC5883.h> // Библиотека поддержтвает оба типа компаса

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


// HMC5883L/QMC5883L Компас
DFRobot_QMC5883 compass;     // Компас

// Screen center
#define CX 120
#define CY 120
#define RADIUS 100
float lastAngle = 0;

void setup() {
  Serial.begin(115200);

// Инициалаизация дисплея
#if (HARDWARE == 120)
  tft.init(240, 240, SPI_MODE3);           // Новая версия
#else  
  tft.init(240, 240, SPI_MODE2);           // Init ST7789 240x240 Параметр SPI_MODE2 ОБЯЗАТЕЛЕН! (без него дисплей не рабоатет!) для версии 110
#endif
  tft.setRotation(1);
  tft.cp437(true);
  tft.fillScreen(ST77XX_BLACK);
  drawCompassFace();
  
  if (!compass.begin()) // попытка инициализировать компас
  {
    Serial.println("Could not find a valid HMC5883L/QMC5883 sensor, check wiring!");
  }    
// в зависимости от чипа настройка компаса и вывод на экран его типа
   if(compass.isHMC()){
        compass.setRange(HMC5883L_RANGE_1_3GA);
        compass.setMeasurementMode(HMC5883L_CONTINOUS);
        compass.setDataRate(HMC5883L_DATARATE_15HZ);
        compass.setSamples(HMC5883L_SAMPLES_8);
    }
   else if(compass.isQMC()){
        compass.setRange(QMC5883_RANGE_2GA);
        compass.setMeasurementMode(QMC5883_CONTINOUS); 
        compass.setDataRate(QMC5883_DATARATE_50HZ);
        compass.setSamples(QMC5883_SAMPLES_8);
   }
 
}

// Draw static compass
void drawCompassFace() {
  tft.drawCircle(CX, CY, RADIUS, ST77XX_WHITE);

  // Cardinal points
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);

  tft.setCursor(CX - 6, CY - RADIUS + 10); tft.print("N");
  tft.setCursor(CX - 6, CY + RADIUS - 20); tft.print("S");
  tft.setCursor(CX + RADIUS - 20, CY - 6); tft.print("E");
  tft.setCursor(CX - RADIUS + 10, CY - 6); tft.print("W");
}

// Draw needle
void drawNeedle(float angle, uint16_t color) {
  float rad = angle * PI / 180.0;

  int x = CX + cos(rad) * (RADIUS - 10);
  int y = CY + sin(rad) * (RADIUS - 10);

  tft.drawLine(CX, CY, x, y, color);
}

void loop() {
// Чтение HMC5883L/
  Vector norm = compass.readNormalize();
  float heading = atan2(norm.YAxis, norm.XAxis); // Calculate heading
  float declinationAngle = (4.0 + (26.0 / 60.0)) / (180 / PI);
  heading += declinationAngle;
  if (heading < 0)     { heading += 2 * PI; }
  if (heading > 2 * PI){ heading -= 2 * PI; }
  int16_t headingDegrees = heading * 180/M_PI; 
 

  // Convert to degrees
  float angle = heading * 180 / PI;

  if (angle < 0) angle += 360;

  // Erase old needle
  drawNeedle(lastAngle, ST77XX_BLACK);

  // Draw new needle
  drawNeedle(angle, ST77XX_RED);

  lastAngle = angle;

  // Display angle
  tft.fillRect(60, 100, 120, 20, ST77XX_BLACK);
  tft.setCursor(70, 105);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.print((int)angle);
  tft.print(" deg");

  delay(100);
}
