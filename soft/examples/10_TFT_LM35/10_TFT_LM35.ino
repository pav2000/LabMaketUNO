// Простейший термометр на датчике LM35 с выводом на дисплей st7789
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#define HARDWARE  120  // Номер версии платы (есть еще 110 версия)
#define VERSION   0.5  // Номер версии ПО
#define BEEP     5     // Пищалка 

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


#define YP A3  
#define XM A2  
#define YM 9   
#define XP 8 

#define XGRAPH 50   // Координаты X термометра
#define YGRAPH 104  // Координаты Y термометра
#define HGRAPH 115  // Высота термометра
#define WGRAPH 19   // ШИРИНА термометра ДОЛЖНА БЫТЬ НЕЧЕТНОЙ

#define LM35 A2


 int newHeight;
 int oldHeight = 0;
 int heightDiff;
                             
void setup(void) {
  Serial.begin(9600);
  Serial.print(F("Hello! LM35 Test"));
 // Инициалаизация дисплея
 #if (HARDWARE == 120)
  tft.init(240, 240, SPI_MODE3);           // Новая версия
#else  
  tft.init(240, 240, SPI_MODE2);           // Init ST7789 240x240 Параметр SPI_MODE2 ОБЯЗАТЕЛЕН! (без него дисплей не рабоатет!) для версии 110
#endif
  tft.setRotation(1);

  tft.fillScreen(ST77XX_BLACK);

  tft.drawRect(10, 10, 220, 75, ST77XX_YELLOW);
  tft.fillRect(15, 15, 210, 65, ST77XX_BLUE);
  tft.drawRect(10, 90, 220, 220, ST77XX_WHITE);
  tft.fillRect(15, 95, 210, 210, ST77XX_WHITE);
  tft.setTextColor(ST77XX_YELLOW, ST77XX_BLUE);
  tft.setTextSize(3);
  tft.setCursor(22, 20);
  tft.print("Thermometer");
  tft.setCursor(22, 50);
  tft.print("Sensor LM35");
  
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(2);
  tft.setCursor(150, 110);
  tft.print("Temp:");

  tft.fillCircle(XGRAPH, YGRAPH, (WGRAPH/2), ST77XX_BLACK);
  tft.fillRect((XGRAPH-(WGRAPH/2)), YGRAPH, WGRAPH, HGRAPH, ST77XX_BLACK);
  tft.fillCircle(XGRAPH, (HGRAPH+YGRAPH), ((WGRAPH/2)+10), ST77XX_BLACK);
  
  tft.fillCircle(XGRAPH, YGRAPH, (WGRAPH/2)-3, ST77XX_WHITE);
  tft.fillRect((XGRAPH-(WGRAPH/2)+3), YGRAPH, (WGRAPH-6), HGRAPH, ST77XX_WHITE);
  tft.fillCircle(XGRAPH, (HGRAPH+YGRAPH), ((WGRAPH/2)+7), ST77XX_WHITE);

  tft.fillCircle((XGRAPH+WGRAPH), YGRAPH, 3, ST77XX_BLACK);
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setCursor(((XGRAPH+WGRAPH)+8), (YGRAPH-5));
  tft.print("100");
  tft.setTextSize(1);
  tft.print("0");
  tft.setTextSize(2);
  tft.print("C");

  tft.fillCircle((XGRAPH+WGRAPH), ((YGRAPH+(HGRAPH/2))-(WGRAPH/2)), 3, ST77XX_BLACK);
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setCursor(((XGRAPH+WGRAPH)+8), (((YGRAPH+(HGRAPH/2))-(WGRAPH/2)))-5);
  tft.print("50");
  tft.setTextSize(1);
  tft.print("0");
  tft.setTextSize(2);
  tft.print("C");
  
  tft.fillCircle((XGRAPH+WGRAPH), ((YGRAPH+HGRAPH)-((WGRAPH/2)+7)), 3, ST77XX_BLACK);
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setCursor(((XGRAPH+WGRAPH)+8), (((YGRAPH+HGRAPH)-((WGRAPH/2)+7))-5));
  tft.print("0");
  tft.setTextSize(1);
  tft.print("0");
  tft.setTextSize(2);
  tft.print("C");
  
  //tft.fillCircle(175, 225, 26, RED);
  tft.fillRect((XGRAPH-(WGRAPH/2)+5), YGRAPH, (WGRAPH-10), HGRAPH, ST77XX_RED);
  tft.fillCircle(XGRAPH, (HGRAPH+YGRAPH), ((WGRAPH/2)+5), ST77XX_RED);
 }

void loop(void) {
 // int suhu=((analogRead(LM35)*5))/10;
   int suhu=0; 
   for(int i=0;i<10;i++){ // Читаем 10 раз
   	suhu=suhu+analogRead(LM35);
   	delay(10);
   }
  suhu=suhu/20;
   
  tft.fillRect(140, 130, 80, 100, ST77XX_WHITE);
  tft.setCursor(140, 130);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(4);
  tft.print(suhu);
  tft.setTextSize(2);
  tft.print("0");
  tft.setTextSize(4);
  tft.print("C");
 // tft.fillRect(140, 130, 80, 4, ST77XX_RED);

  if(suhu <= 100){
    newHeight = map(suhu,0, 100, 0, (HGRAPH-(WGRAPH/2)));
    heightDiff = oldHeight-newHeight; // рисовать только новую часть гистограммы для более быстрого отображения
    if (oldHeight != newHeight) tft.fillRect((XGRAPH-(WGRAPH/2)+5), YGRAPH+((HGRAPH-suhu)-((WGRAPH/2)+7)), (WGRAPH-10), HGRAPH-((HGRAPH-suhu)-((WGRAPH/2)+7)), ST77XX_RED);
    tft.fillRect((XGRAPH-(WGRAPH/2)+5), YGRAPH, (WGRAPH-10), ((HGRAPH-suhu)-((WGRAPH/2)+7)), ST77XX_WHITE);
    if (suhu==100)tft.fillCircle(XGRAPH, YGRAPH, (WGRAPH/2)-5, ST77XX_RED);
    oldHeight=newHeight; // remember how high bar is
  }
  tft.fillCircle(XGRAPH, (HGRAPH+YGRAPH), ((WGRAPH/2)+5), ST77XX_RED);
  delay(500);
}
