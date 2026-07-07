/***********************************************************************
 * 
 * Interfacing Arduino with ST7789 TFT display (240x240 pixel)
 *   and DHT11 digital humidity & temperature sensor.
 * This is a free software with NO WARRANTY.
 * http://simple-circuit.com/
 *
 ***********************************************************************/
 
#include <Adafruit_GFX.h>     // Adafruit core graphics library
#include <Adafruit_ST7789.h>  // Adafruit hardware-specific library for ST7789
#include <DHT.h>              // Adafruit DHT library code
 
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
  #define TFT_DC        9  // КомандыVL53L0X
#endif  
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);  // Объявление дисплея

#define DHTPIN  4           // DHT11 data pin is connected to Arduino  pin 4
#define DHTTYPE DHT11        // DHT11 sensor is used
DHT dht11(DHTPIN, DHTTYPE);  // initialize DHT library
 
void setup(void) {
  
 // Инициалаизация дисплея
 #if (HARDWARE == 120)
  tft.init(240, 240, SPI_MODE3);           // Новая версия
#else  
  tft.init(240, 240, SPI_MODE2);           // Init ST7789 240x240 Параметр SPI_MODE2 ОБЯЗАТЕЛЕН! (без него дисплей не рабоатет!) для версии 110
#endif

   tft.setRotation(1);
  
  // SPI speed defaults to SPI_DEFAULT_FREQ defined in the library, you can override it here
  // Note that speed allowable depends on chip and quality of wiring, if you go too fast, you
  // may end up with a black screen some times, or all the time.
  //tft.setSPISpeed(40000000);

  tft.fillScreen(0); // Clear screen
  tft.setTextWrap(false);                        // turn off text wrap option
  tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);  // set text color to green and black background
  tft.setTextSize(3);           // text size = 3
  tft.setCursor(15, 40);        // move cursor to position (15, 27) pixel
  tft.print("TEMPERATURE:");
  tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);  // set text color to yellow and black background
  tft.setCursor(43, 140);    // move cursor to position (15, 27) pixel
  tft.print("HUMIDITY:");
  tft.setTextSize(4);        // text size = 4
 
  // initialize DHT11 sensor
  dht11.begin();
 
}
 
char _buffer[7];
// main loop
void loop() {
 
  delay(1000);    // wait a second
 
  // read humidity in rH%
  int Humi = dht11.readHumidity() * 10;
  // read temperature in degrees Celsius
  int Temp = dht11.readTemperature() * 10;
 
  // print temperature (in °C)
  tft.setTextColor(ST77XX_RED, ST77XX_BLACK);  // set text color to red with black background
  if(Temp < 0)    // if temperature < 0
    sprintf(_buffer, "-%02u.%1u", (abs(Temp)/10)%100, abs(Temp) % 10);
  else            // temperature >= 0
    sprintf(_buffer, " %02u.%1u", (Temp/10)%100, Temp % 10);
  tft.setCursor(26, 71);
  tft.print(_buffer);
  tft.drawCircle(161, 77, 4, ST77XX_RED);  // print degree symbol ( ° )
  tft.drawCircle(161, 77, 5, ST77XX_RED);
  tft.setCursor(170, 71);
  tft.print("C");
 
  // print humidity (in %)
  tft.setTextColor(ST77XX_CYAN, ST77XX_BLACK);  // set text color to cyan and black background
  sprintf(_buffer, "%02u.%1u %%", (Humi/10)%100, Humi % 10);
  tft.setCursor(50, 171);
  tft.print(_buffer);
 
}
 
// end of code.
