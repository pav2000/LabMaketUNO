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
#include <HX711.h>
 
#define HARDWARE  120  // Номер версии платы (есть еще 110 версия)
#define VERSION   0.5  // Номер версии ПО

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

#define DOUT 8
#define CLK  7
HX711 scale;
float calibration_factor = 420.0; // Подберите при калибровке
 
void setup(void) {
   Serial.begin(115200);
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
  tft.setCursor(1, 2*24);        // move cursor to position (15, 27) pixel
  tft.print("ADC:");
  tft.setCursor(1, 3*24);        // move cursor to position (15, 27) pixel
  tft.print("avr20:");
  tft.setCursor(1, 4*24);        // move cursor to position (15, 27) pixel
  tft.print("Bec :");
  tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);  // set text color to yellow and black background

  
  scale.begin(DOUT, CLK); // Датчик
  scale.set_scale(calibration_factor); // Коэффициент калибровки
  scale.tare(); // Обнуление весов
 }
 
char _buffer[7];
void loop() {
  if (scale.is_ready()) {

    // Значение ADC
    uint32_t adc = scale.read(); 
    tft.setTextColor(ST77XX_RED, ST77XX_BLACK); 
    sprintf(_buffer, "%ld", adc);
    tft.fillRect(100, 2*24, 130, 24, ST77XX_BLACK);
    tft.setCursor(100, 2*24);
    tft.print(_buffer);
 
    // Значение усреднения ADC
    uint32_t average = scale.read_average(20); 
    tft.setTextColor(ST77XX_RED, ST77XX_BLACK); 
    sprintf(_buffer, "%ld", average);
    tft.fillRect(110, 3*24, 130, 24, ST77XX_BLACK);
    tft.setCursor(110, 3*24);
    tft.print(_buffer);

    // Получаем значение массы в граммах
    float weight = scale.get_units(10);
    tft.setTextColor(ST77XX_RED, ST77XX_BLACK); 
    dtostrf(weight, 6, 2, _buffer);  // выводим в строку buf 2 разряда до, 2 разряда после запятой
    tft.fillRect(110, 4*24, 130, 24, ST77XX_BLACK);
    tft.setCursor(110, 4*24);
    tft.print(_buffer);
    
  } 
  else  Serial.println("HX711 не найден");
  delay(100);
 }
 
// end of code.
