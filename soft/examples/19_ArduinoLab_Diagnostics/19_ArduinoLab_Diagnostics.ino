/*
  ==========================================================================
  ArduinoLab — диагностический скетч
  Плата: ArduinoLab (Pav2000), rev 1.2, 2025-08-19
  ==========================================================================

  Скетч выводит на TFT-экран (ST7789 1.3", 240x240) МЕНЮ выбора теста.
  При старте показывается заставка: версия прошивки, объём свободной RAM и
  результаты сканирования шины I2C с адресами найденных устройств (пауза
  5 секунд), после чего появляется меню.
  Навигация — кнопками на A3 (резистивная лесенка SW1/SW2/SW5):
    SW1 — вверх по меню
    SW5 — вниз по меню
    SW2 — выбрать / запустить выделенный тест
  Результат выбранного теста выводится на экран и дублируется в Serial
  (115200). Любая кнопка после теста возвращает в меню. Циклической
  автоматической проверки всех узлов подряд больше нет.

  ------------------------------ КОМПОНЕНТЫ -------------------------------
  U1   1117-3.3           — стабилизатор 3.3В (питание, отдельно не тестируется)
  TFT1 ST7789 1.3"        — экран, SPI через TXS0108 (5В -> 3.3В); собственный
                             минимальный драйвер (см. ниже) — без Adafruit_GFX/
                             ST7789/BusIO
  U7   GY-273 HMC5883L    — 3-осевой компас, I2C 0x1E (или 0x0D, если реально
                             стоит клон QMC5883L — определяется автоматически,
                             драйвер написан на прямых регистрах, без библиотек)
  U14  DS3232MZ+TRL       — часы реального времени (RTC), I2C 0x68, батарея BT1
                             (тоже прямые регистры, без библиотеки)
  U13  CAT24C32WI-G       — EEPROM 32Кбит, I2C (адрес зависит от A0..A2, обычно 0x50)
  U11  DHT11              — датчик температуры/влажности, пин D4
  U10  LM35DZ             — аналоговый датчик температуры, пин A2
  R9   GL5549 (LDR)       — фоторезистор (освещённость), пин A1
  U16  HC-12              — радиомодуль 433МГц, SoftwareSerial D7(TX)/D8(RX),
                             SET управляется с D6 (перемычки H1 в норме 3-5 и 4-6)
  SW3, SW4                — кнопки на D2 / D3 (цифровые, INPUT_PULLUP)
  SW1, SW2, SW5           — резистивная "лесенка" кнопок на A3 (аналоговая)
  Q1 + SG1                — пьезозуммер, управление транзистором BC807 с D5
  U9   LM385Z-2.5         — источник опорного напряжения 2.5В на AREF
  LED1/LED2                — индикаторы активности на линиях SCK/MISO (пассивные)
  U15  TC5050RGBF08 (RGB) — индикатор активности на линиях DC/RES/MOSI (пассивный)
  LED3                     — индикатор питания HC-12 (пассивный)

  U12  VL53L0X            — лазерный дальномер, I2C 0x29 (библиотека Pololu
                             VL53L0X — компактнее Adafruit_VL53L0X, не тянет
                             Adafruit_BusIO)

  Тесты компаса, DHT11, LM35 и фоторезистора работают в режиме "живого"
  просмотра: значения обновляются раз в 1 секунду, выход в меню — по любой
  кнопке (не дожидаясь окончания цикла).

  Экран переведён на собственный минимальный SPI-драйвер (без Adafruit_GFX/
  ST7789/BusIO). 
  ------------------------- НУЖНЫЕ БИБЛИОТЕКИ ------------------------------
  (Скетч -> Подключить библиотеку -> Управлять библиотеками)
    - DHT11 (Dhruba Saha, dhrubasaha08/DHT11)  — вместо Adafruit DHT sensor library
    - VL53L0X (Pololu, pololu/vl53l0x-arduino) — вместо Adafruit_VL53L0X
  Wire, SPI, SoftwareSerial — входят в состав Arduino IDE.
  Adafruit_GFX, Adafruit_ST7789, Adafruit_BusIO, Adafruit_VL53L0X, DS3231
  (NorthernWidget), DFRobot_QMC5883 — НЕ используются, исключены из проекта
  для экономии флеш/RAM. Компас (HMC5883L/QMC5883L) и часы (DS3232) работают
  напрямую через регистры по I2C (см. compassBegin()/compassRead() и
  ds3232ReadTime() ниже) — без сторонних библиотек вообще.

  --------------------------- ВАЖНЫЕ ДОПУЩЕНИЯ ------------------------------
    TFT_RST = 10, TFT_DC = 9, CS — не подключен (модуль сажает его на GND,
    свой драйвер этот пин вообще не использует). Подсветка (BLK) всегда
    включена аппаратно и кодом не управляется.
  SET модуля HC-12 подключается через перемычку H1 (позиции 3-5 и 4-6
  в нормальном состоянии) к пину D6 и может программно устанавливаться
  как в 0 (режим AT-команд), так и в 1 (обычный режим передачи).
  Кнопки SW1/SW2/SW5 подключены резистивной цепочкой к A3 (лесенка).
  Функционально: SW1 = UP (вверх по меню), SW2 = OK (выбор), SW5 = DOWN
  (вниз по меню). Пороговые значения ADC — по результатам реального
  тестирования платы (см. readLadderRaw()):
    нет нажатий -> ADC < 200   -> none
    DOWN (SW5)  -> 200..399    -> Down
    OK   (SW2)  -> 400..699    -> Ok
    UP   (SW1)  -> >= 700      -> Up
  ==========================================================================
*/

#include <SPI.h>
#include <Wire.h>
#include <DHT11.h>
#include <VL53L0X.h>
#include <SoftwareSerial.h>

// ============================== PIN CONFIG ================================
#define TFT_RST   10
#define TFT_DC    9
// CS у модуля не разведён на Arduino — считаем посаженным на GND, свой драйвер
// пин CS вообще не использует.
// BLK (подсветка) всегда включена аппаратно, GPIO не требуется и не используется

#define PIN_DHT       4

#define PIN_BTN_D2    2   // SW3
#define PIN_BTN_D3    3   // SW4
#define PIN_BTN_LADDER A3 // SW1 / SW2 / SW5

#define PIN_LDR       A1  // GL5549
#define PIN_LM35      A2  // LM35DZ
#define PIN_SPARE_A0  A0  // резервный аналоговый вход

#define PIN_BUZZER    5           // база Q1 (BC807, PNP)
#define BUZZER_ON     LOW         // PNP открывается по низкому уровню на базе
#define BUZZER_OFF    HIGH

#define HC12_ARDUINO_TX 7  // Arduino D7  -> HC-12 RX  (сеть "softTX")
#define HC12_ARDUINO_RX 8  // Arduino D8  <- HC-12 TX  (сеть "softRX")
#define PIN_HC12_SET    6  // Arduino D6  -> HC-12 SET, через перемычку H1 (3-5/4-6)
#define HC12_MODE_NORMAL HIGH  // SET = 1: обычный режим передачи данных
#define HC12_MODE_AT     LOW   // SET = 0: режим AT-команд (настройка модуля)

#define I2C_ADDR_HMC5883L 0x1E
#define I2C_ADDR_QMC5883L 0x0D   // многие модули GY-273 на деле содержат клон QMC5883L
#define I2C_ADDR_DS3232   0x68
#define I2C_ADDR_EEPROM   0x50   // адрес по умолчанию, если A0..A2 EEPROM на GND

#define FW_VERSION "v1.5"

// Параметры SPI/адресации ST7789, подобранные экспериментально для этой
// партии модулей (см. историю чата — перебор режимов SPI и офсетов):
//   - SPI_MODE3 (не MODE0!) — конкретно этот экран требует именно его
//     (в исходниках Adafruit_ST77xx.h есть заводской комментарий:
//     "certain display needs MODE3 instead")
//   - MADCTL = 0xA0 (биты MY|MV) и смещение +80 по X — соответствует
//     rotation=1 в терминах Adafruit_ST7789 (GRAM контроллера 240x320,
//     видимая матрица 240x240 занимает только часть этой памяти)
//   - инициализация выполняется ДВАЖДЫ подряд: на практике одного прохода
//     сразу после подачи питания оказалось недостаточно для стабильного
//     запуска этой партии модулей, повтор полностью решает проблему
#define TFT_SPI_SPEED_HZ 8000000UL
#define TFT_SPI_MODE     SPI_MODE3
#define TFT_MADCTL       0xA0
#define TFT_X_OFFSET     80
#define TFT_Y_OFFSET     0

// ========================== СВОЙ ДРАЙВЕР ST7789 =============================
// Заменяет Adafruit_GFX + Adafruit_ST7789 (которые тянут Adafruit_BusIO).
// Только то, что реально используется в этом скетче: заливка прямоугольников,
// линии и текст компактным шрифтом 5x7 (только заглавные буквы — экономия
// флеша; строчные буквы автоматически переводятся в заглавные при выводе).

// Компактный шрифт 5x7, только заглавные буквы/цифры/пунктуация (см. FONT_CHARS).
const char FONT_CHARS[] PROGMEM = " !%()+,-./0123456789:=ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const uint8_t FONT_DATA[48][5] PROGMEM = {
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 'SPACE'
  { 0x00, 0x2F, 0x00, 0x00, 0x00 }, // '!'
  { 0x63, 0x10, 0x08, 0x26, 0x41 }, // '%'
  { 0x1C, 0x22, 0x41, 0x00, 0x00 }, // '('
  { 0x00, 0x00, 0x41, 0x22, 0x1C }, // ')'
  { 0x08, 0x08, 0x3E, 0x08, 0x08 }, // '+'
  { 0x40, 0x20, 0x00, 0x00, 0x00 }, // ','
  { 0x08, 0x08, 0x08, 0x08, 0x08 }, // '-'
  { 0x60, 0x60, 0x00, 0x00, 0x00 }, // '.'
  { 0x10, 0x08, 0x04, 0x02, 0x01 }, // '/'
  { 0x3E, 0x51, 0x49, 0x45, 0x3E }, // '0'
  { 0x00, 0x42, 0x7F, 0x40, 0x00 }, // '1'
  { 0x42, 0x61, 0x51, 0x49, 0x46 }, // '2'
  { 0x21, 0x41, 0x49, 0x4D, 0x32 }, // '3'
  { 0x18, 0x14, 0x12, 0x7F, 0x10 }, // '4'
  { 0x27, 0x45, 0x45, 0x45, 0x39 }, // '5'
  { 0x3C, 0x4A, 0x49, 0x49, 0x30 }, // '6'
  { 0x01, 0x71, 0x09, 0x05, 0x03 }, // '7'
  { 0x36, 0x49, 0x49, 0x49, 0x36 }, // '8'
  { 0x06, 0x09, 0x49, 0x69, 0x1E }, // '9'
  { 0x00, 0x00, 0x36, 0x00, 0x00 }, // ':'
  { 0x0A, 0x0A, 0x0A, 0x0A, 0x0A }, // '='
  { 0x7E, 0x09, 0x09, 0x09, 0x7E }, // 'A'
  { 0x7F, 0x49, 0x49, 0x49, 0x36 }, // 'B'
  { 0x3E, 0x41, 0x41, 0x41, 0x22 }, // 'C'
  { 0x7F, 0x41, 0x41, 0x41, 0x3E }, // 'D'
  { 0x7F, 0x49, 0x49, 0x49, 0x41 }, // 'E'
  { 0x7F, 0x09, 0x09, 0x09, 0x01 }, // 'F'
  { 0x3E, 0x41, 0x49, 0x49, 0x3A }, // 'G'
  { 0x7F, 0x08, 0x08, 0x08, 0x7F }, // 'H'
  { 0x00, 0x41, 0x7F, 0x41, 0x00 }, // 'I'
  { 0x20, 0x40, 0x40, 0x41, 0x3F }, // 'J'
  { 0x7F, 0x08, 0x14, 0x22, 0x41 }, // 'K'
  { 0x7F, 0x40, 0x40, 0x40, 0x40 }, // 'L'
  { 0x7F, 0x02, 0x04, 0x02, 0x7F }, // 'M'
  { 0x7F, 0x02, 0x04, 0x08, 0x7F }, // 'N'
  { 0x3E, 0x41, 0x41, 0x41, 0x3E }, // 'O'
  { 0x7F, 0x09, 0x09, 0x09, 0x06 }, // 'P'
  { 0x3E, 0x41, 0x51, 0x21, 0x5E }, // 'Q'
  { 0x7F, 0x09, 0x19, 0x29, 0x46 }, // 'R'
  { 0x46, 0x49, 0x49, 0x49, 0x31 }, // 'S'
  { 0x01, 0x01, 0x7F, 0x01, 0x01 }, // 'T'
  { 0x3F, 0x40, 0x40, 0x40, 0x3F }, // 'U'
  { 0x1F, 0x20, 0x40, 0x20, 0x1F }, // 'V'
  { 0x3F, 0x40, 0x38, 0x40, 0x3F }, // 'W'
  { 0x63, 0x14, 0x08, 0x14, 0x63 }, // 'X'
  { 0x03, 0x04, 0x78, 0x04, 0x03 }, // 'Y'
  { 0x61, 0x51, 0x49, 0x45, 0x43 }, // 'Z'
};
#define FONT_COUNT 48

class MiniTFT {
public:
  void init(uint16_t w, uint16_t h) {
    _w = w; _h = h;
    pinMode(TFT_DC, OUTPUT);
    pinMode(TFT_RST, OUTPUT);

 //   delay(200); // стабилизация питания после старта — критично для этой платы

    SPI.begin();
    SPI.beginTransaction(SPISettings(TFT_SPI_SPEED_HZ, MSBFIRST, TFT_SPI_MODE));

    // Инициализация выполняется дважды — экспериментально подтверждено,
    // что одного прохода сразу после включения питания недостаточно.
    initSequence();
  //  initSequence();
  }

  void setRotation(uint8_t) { /* поддерживается только найденная рабочая ориентация */ }

  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (x >= _w || y >= _h || w <= 0 || h <= 0) return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > _w) w = _w - x;
    if (y + h > _h) h = _h - y;
    if (w <= 0 || h <= 0) return;

    setAddrWindow(x, y, x + w - 1, y + h - 1);
    digitalWrite(TFT_DC, HIGH);
    uint8_t hi = color >> 8, lo = color & 0xFF;
    uint32_t n = (uint32_t)w * (uint32_t)h;
    for (uint32_t i = 0; i < n; i++) { SPI.transfer(hi); SPI.transfer(lo); }
  }

  void fillScreen(uint16_t color) { fillRect(0, 0, _w, _h, color); }
  void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t /*r*/, uint16_t color) {
    fillRect(x, y, w, h, color); // радиус игнорируем — экономим флеш
  }
  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) { fillRect(x, y, w, 1, color); }

  void setTextColor(uint16_t fg) { _fg = fg; _useBg = false; }
  void setTextColor(uint16_t fg, uint16_t bg) { _fg = fg; _bg = bg; _useBg = true; }
  void setCursor(int16_t x, int16_t y) { _cx = x; _cy = y; }
  void setTextSize(uint8_t s) { _size = (s == 0) ? 1 : s; }
  void setTextWrap(bool) { /* перенос строк не поддерживается — не требуется */ }

  void print(const String &s) {
    for (uint16_t i = 0; i < s.length(); i++) {
      char c = s[i];
      if (c == '\n') { _cx = 0; _cy += 8 * _size; continue; }
      drawChar(_cx, _cy, c);
      _cx += 6 * _size;
    }
  }
  void print(const char* s)     { print(String(s)); }
  void print(const __FlashStringHelper* s) { print(String(s)); } // строки из PROGMEM (F())
  void print(char c)            { char b[2] = { c, 0 }; print(String(b)); }
  void print(int n)             { print(String(n)); }
  void print(unsigned int n)    { print(String(n)); }
  void print(unsigned char n, int base) { print(String(n, base)); }

  void println(const String &s) { print(s); print('\n'); }
  void println(const char* s)   { println(String(s)); }
  void println(const __FlashStringHelper* s) { println(String(s)); } // строки из PROGMEM (F())
  void println(int n)           { println(String(n)); }
  void println(unsigned char n) { println(String(n)); }

private:
  int16_t _w = 0, _h = 0;
  int16_t _cx = 0, _cy = 0;
  uint16_t _fg = 0xFFFF, _bg = 0x0000;
  uint8_t _size = 1;
  bool _useBg = false;

  void cmd(uint8_t c) { digitalWrite(TFT_DC, LOW);  SPI.transfer(c); }
  void dat(uint8_t d) { digitalWrite(TFT_DC, HIGH); SPI.transfer(d); }
  void dat16(uint16_t d) { digitalWrite(TFT_DC, HIGH); SPI.transfer(d >> 8); SPI.transfer(d & 0xFF); }

  void initSequence() {
    digitalWrite(TFT_RST, HIGH); delay(10);
    digitalWrite(TFT_RST, LOW);  delay(20);
    digitalWrite(TFT_RST, HIGH); delay(150);

    cmd(0x01); delay(150);           // SWRESET
    cmd(0x11); delay(120);           // SLPOUT
    cmd(0x3A); dat(0x55); delay(10); // COLMOD: 16 бит/пиксель
    cmd(0x36); dat(TFT_MADCTL);      // MADCTL: 0xA0 (MY|MV) — найдено экспериментально
    cmd(0x21); delay(10);            // INVON
    cmd(0x13); delay(10);            // NORON
    cmd(0x29); delay(10);            // DISPON
  }

  void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    x0 += TFT_X_OFFSET; x1 += TFT_X_OFFSET; // смещение GRAM (240x320 -> видимые 240x240)
    y0 += TFT_Y_OFFSET; y1 += TFT_Y_OFFSET;
    cmd(0x2A); dat16(x0); dat16(x1); // CASET
    cmd(0x2B); dat16(y0); dat16(y1); // RASET
    cmd(0x2C);                       // RAMWR
  }

  void drawChar(int16_t x, int16_t y, char c) {
    if (c >= 'a' && c <= 'z') c -= 32; // приводим к верхнему регистру
    int8_t idx = -1;
    for (uint8_t i = 0; i < FONT_COUNT; i++) {
      if ((char)pgm_read_byte(&FONT_CHARS[i]) == c) { idx = i; break; }
    }
    if (idx < 0) return; // неизвестный символ — пропускаем (место всё равно сдвинется)

    for (uint8_t col = 0; col < 5; col++) {
      uint8_t line = pgm_read_byte(&(FONT_DATA[idx][col]));
      for (uint8_t row = 0; row < 7; row++) {
        bool on = line & (1 << row);
        if (!on && !_useBg) continue; // прозрачный фон — «пустые» пиксели не трогаем
        uint16_t color = on ? _fg : _bg;
        if (_size == 1) fillRect(x + col, y + row, 1, 1, color);
        else fillRect(x + col * _size, y + row * _size, _size, _size, color);
      }
    }
  }
};

// цвета (значения RGB565, совпадают с прежними ST77XX_* из Adafruit_ST7789)
#define ST77XX_BLACK   0x0000
#define ST77XX_WHITE   0xFFFF
#define ST77XX_RED     0xF800
#define ST77XX_GREEN   0x07E0
#define ST77XX_BLUE    0x001F
#define ST77XX_CYAN    0x07FF
#define ST77XX_MAGENTA 0xF81F
#define ST77XX_YELLOW  0xFFE0

#define COL_BG     ST77XX_BLACK
#define COL_TITLE  ST77XX_CYAN
#define COL_OK     ST77XX_GREEN
#define COL_FAIL   ST77XX_RED
#define COL_TEXT   ST77XX_WHITE
#define COL_WARN   ST77XX_YELLOW
#define COL_GREY   0x7BEF  // тёмно-серый (RGB565)

// ============================== ОБЪЕКТЫ ====================================
MiniTFT tft;                     // свой драйвер ST7789 (без Adafruit_GFX/BusIO)
DHT11 dhtSensor(PIN_DHT);        // библиотека DHT11 (Dhruba Saha) — пин задаётся в конструкторе
VL53L0X distanceSensor;          // библиотека Pololu VL53L0X (без Adafruit_BusIO)
SoftwareSerial hc12(HC12_ARDUINO_RX, HC12_ARDUINO_TX);

// ============================== МЕНЮ ========================================
// Навигация меню — кнопками A3 (резистивная лесенка, см. readLadderRaw()):
//   SW1 — вверх по меню, SW5 — вниз по меню, SW2 — выбрать/запустить тест.

typedef void (*TestFunc)();

// Явные прототипы тестовых функций — обязательны для массива указателей ниже.
void testTFT();
void testCompassHMC5883L();
void testRTC_DS3232();
void testEEPROM_AT24C32();
void testDHT11();
void testLM35();
void testLDR();
void testSpareA0();
void testDigitalButtons();
void testDistanceVL53L0X();
void testBuzzer();
void testHC12();

// Прототип нужен, т.к. printExitHint() (определена ниже) вызывает printLine(),
// а printLine() определена значительно дальше по файлу — эта среда сборки
// не генерирует прототипы автоматически.
void printLine(int y, const String &s, uint16_t color = COL_TEXT, uint8_t size = 2);

// Прототип нужен по той же причине: setup() вызывает splash(), а splash()
// определена значительно дальше по файлу.
void splash();

struct MenuItem {
  const char* name; // указатель на строку в PROGMEM (см. MENU_NAME_* ниже)
  TestFunc func;
};

// Названия пунктов меню — во флеш-памяти (PROGMEM)
const char MENU_NAME_1[]  PROGMEM = "1  TFT ST7789";
const char MENU_NAME_2[]  PROGMEM = "2  Compass HMC5883L";
const char MENU_NAME_3[]  PROGMEM = "3  RTC DS3232";
const char MENU_NAME_4[]  PROGMEM = "4  EEPROM AT24C32";
const char MENU_NAME_5[]  PROGMEM = "5  DHT11";
const char MENU_NAME_6[]  PROGMEM = "6  LM35 (temp.)";
const char MENU_NAME_7[]  PROGMEM = "7  Photoresistor LDR";
const char MENU_NAME_8[]  PROGMEM = "8  Variable Resistor";
const char MENU_NAME_9[]  PROGMEM = "9  Buttons D2/D3";
const char MENU_NAME_10[] PROGMEM = "10 Distance VL53L0X";
const char MENU_NAME_11[] PROGMEM = "11 Buzzer";
const char MENU_NAME_12[] PROGMEM = "12 HC-12 (radio)";

MenuItem menuItems[] = {
  {MENU_NAME_1,  testTFT},
  {MENU_NAME_2,  testCompassHMC5883L},
  {MENU_NAME_3,  testRTC_DS3232},
  {MENU_NAME_4,  testEEPROM_AT24C32},
  {MENU_NAME_5,  testDHT11},
  {MENU_NAME_6,  testLM35},
  {MENU_NAME_7,  testLDR},
  {MENU_NAME_8,  testSpareA0},
  {MENU_NAME_9,  testDigitalButtons},
  {MENU_NAME_10, testDistanceVL53L0X},
  {MENU_NAME_11, testBuzzer},
  {MENU_NAME_12, testHC12},
};
const int NUM_MENU_ITEMS = sizeof(menuItems) / sizeof(menuItems[0]);

int menuSelected = 0;

// Пороговые значения ADC для лесенки кнопок A3 — присланы пользователем по
// результатам реального тестирования платы 
enum LadderBtn { LB_NONE, LB_DOWN, LB_OK, LB_UP };

LadderBtn readLadderRaw() {
  int key = analogRead(PIN_BTN_LADDER);
  if (key < 200) return LB_NONE;
  else if (key < 400) return LB_DOWN;
  else if (key < 700) return LB_OK;
  else return LB_UP;
}

// Блокирующее ожидание нажатия (с антидребезгом). Сначала ждём отпускания
// (если кнопка ещё держится с прошлого раза), затем — новое нажатие.
LadderBtn waitLadderPress() {
  LadderBtn s;
  do { s = readLadderRaw(); } while (s != LB_NONE);

  LadderBtn pressed;
  while (true) {
    pressed = readLadderRaw();
    if (pressed != LB_NONE) {
      delay(30); // антидребезг
      if (readLadderRaw() == pressed) break;
    }
  }
  return pressed;
}

// Ждать отпускания кнопки (если она ещё держится с прошлого раза — например,
// сразу после входа в тест по кнопке OK). Используется перед началом "живых"
// тестов с автообновлением, чтобы уже нажатая кнопка не сработала как выход.
void waitLadderRelease() {
  while (readLadderRaw() != LB_NONE) delay(10);
}

// Пауза до ms миллисекунд, которая прерывается раньше, если нажата любая
// кнопка. Возвращает true, если пауза была прервана нажатием (выход из
// "живого" теста), false — если истекло полное время (пора обновить данные).
bool liveWait(unsigned long ms) {
  unsigned long t0 = millis();
  while (millis() - t0 < ms) {
    if (readLadderRaw() != LB_NONE) return true;
    delay(30);
  }
  return false;
}

// "Живые" тесты (с автообновлением) сами ждут нажатие кнопки, чтобы выйти —
// одного нажатия достаточно. Чтобы runSelectedTest() не требовал ВТОРОЕ
// нажатие для возврата в меню, тест перед выходом ставит этот флаг.
bool skipReturnWait = false;

// Печатает один раз (не в цикле обновления) подсказку внизу экрана для
// "живых" тестов — сразу видно, как выйти.
void printExitHint() {
  printLine(207, F("Any button = exit"), COL_GREY, 1);
}

// Y-координата строки пункта меню номер i (используется и полной, и
// точечной перерисовкой, чтобы координаты никогда не разъезжались).
const int MENU_ITEM_Y0 = 30;
const int MENU_LINE_H  = 16;
inline int menuItemY(int i) { return MENU_ITEM_Y0 + i * MENU_LINE_H; }

// Перерисовывает ОДИН пункт меню (фон + текст). Используется как при полной
// отрисовке меню, так и при точечном обновлении (Up/Down) — тогда
// перерисовываются только две затронутые строки, а не весь экран.
void drawMenuItem(int i, bool selected) {
  int y = menuItemY(i);
  tft.setTextSize(1);
  if (selected) {
    tft.fillRect(0, y - 1, 240, MENU_LINE_H, ST77XX_BLUE);
    tft.setTextColor(ST77XX_WHITE);
  } else {
    tft.fillRect(0, y - 1, 240, MENU_LINE_H, COL_BG);
    tft.setTextColor(COL_TEXT);
  }
  tft.setCursor(6, y + 2);
  tft.println((const __FlashStringHelper*)menuItems[i].name);
}

// Полная перерисовка экрана меню — вызывается только при входе в меню
// (первый запуск / возврат из теста), НЕ на каждое нажатие Up/Down.
void drawMenuFull() {
  tft.fillScreen(COL_BG);
  tft.setTextColor(COL_TITLE);
  tft.setTextSize(2);
  tft.setCursor(6, 4);
  tft.println(F("TEST MENU"));
  tft.drawFastHLine(0, 24, 240, COL_GREY);

  for (int i = 0; i < NUM_MENU_ITEMS; i++) {
    drawMenuItem(i, i == menuSelected);
  }

  int y = menuItemY(NUM_MENU_ITEMS);
  tft.drawFastHLine(0, y + 2, 240, COL_GREY);
  tft.setTextColor(COL_WARN);
  tft.setTextSize(1);
  tft.setCursor(4, y + 8);
  tft.println(F("UP/DOWN - select, OK - start"));
}

void runSelectedTest() {
  menuItems[menuSelected].func();

  if (skipReturnWait) {
    // "живой" тест уже вышел по одному нажатию — второе не требуется
    skipReturnWait = false;
    return;
  }

  // футер-подсказка и ожидание возврата в меню (по нажатию любой кнопки)
  tft.fillRect(0, 220, 240, 20, COL_BG);
  tft.setTextColor(COL_GREY);
  tft.setTextSize(1);
  tft.setCursor(6, 224);
  tft.println(F("Any button - menu"));
  waitLadderPress();
}

// ============================== SETUP ======================================
void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println(F("=== ArduinoLab: diagnostics started ==="));

  pinMode(PIN_BTN_D2, INPUT_PULLUP);
  pinMode(PIN_BTN_D3, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, BUZZER_OFF);
  pinMode(PIN_HC12_SET, OUTPUT);
  digitalWrite(PIN_HC12_SET, HC12_MODE_NORMAL); // по умолчанию — обычный режим передачи

  Wire.begin();

  tft.init(240, 240);
  tft.setRotation(0);
  tft.fillScreen(COL_BG);

  hc12.begin(9600);

  splash();     // версия, свободная RAM, скан I2C, пауза 5с — затем меню
}

// ============================== LOOP =======================================
// Полная перерисовка — только при входе в меню; Up/Down точечно обновляют
// только две затронутые строки (старый и новый выбранный пункт).
void loop() {
  drawMenuFull();

  while (true) {
    LadderBtn btn = waitLadderPress();

    if (btn == LB_UP) {
      int old = menuSelected;
      menuSelected = (menuSelected - 1 + NUM_MENU_ITEMS) % NUM_MENU_ITEMS;
      drawMenuItem(old, false);
      drawMenuItem(menuSelected, true);
    } else if (btn == LB_DOWN) {
      int old = menuSelected;
      menuSelected = (menuSelected + 1) % NUM_MENU_ITEMS;
      drawMenuItem(old, false);
      drawMenuItem(menuSelected, true);
    } else if (btn == LB_OK) {
      runSelectedTest();
      break; // после теста — полная перерисовка меню в начале loop()
    }
    // LB_NONE в меню игнорируется
  }
}

// ============================ ОБЩИЕ ФУНКЦИИ =================================

void screenHeader(const __FlashStringHelper* title) {
  tft.fillScreen(COL_BG);
  tft.setTextWrap(true);
  tft.setTextColor(COL_TITLE);
  tft.setTextSize(2);
  tft.setCursor(6, 8);
  tft.println(title);
  tft.drawFastHLine(0, 30, 240, COL_GREY);
}

void printLine(int y, const String &s, uint16_t color = COL_TEXT, uint8_t size = 2) {
  tft.setTextSize(size);
  tft.setTextColor(color, COL_BG);
  tft.setCursor(6, y);
  tft.println(s);
}

void statusBadge(bool ok, int y) {
  tft.fillRoundRect(6, y, 100, 26, 4, ok ? COL_OK : COL_FAIL);
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setCursor(14, y + 5);
  tft.print(ok ? F("OK") : F("FAIL"));
}

void logResult(const __FlashStringHelper* name, bool ok, const String &details) {
  Serial.print(F("["));
  Serial.print(ok ? F("OK") : F("FAIL"));
  Serial.print(F("] "));
  Serial.print(name);
  Serial.print(F(" -> "));
  Serial.println(details);
}

// Возвращает объём свободной оперативной памяти (SRAM) в байтах (стандартный
// приём для AVR: разница между вершиной кучи и текущим стеком).
int freeRam() {
  extern int __heap_start, *__brkval;
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

// Человекочитаемое имя устройства по известному адресу I2C на этой плате.
String i2cDeviceName(byte addr) {
  switch (addr) {
    case I2C_ADDR_HMC5883L: return F("HMC5883L (compass)");
    case I2C_ADDR_QMC5883L: return F("QMC5883L (compass)");
    case 0x29:              return F("VL53L0X (distance)");
    case I2C_ADDR_EEPROM:   return F("AT24C32 (EEPROM)");
    case I2C_ADDR_DS3232:   return F("DS3232 (RTC)");
    default:                return F("");
  }
}

// Стартовый экран: версия прошивки, свободная RAM, результаты сканирования
// шины I2C с адресами устройств. Пауза 5 секунд, затем переход в меню.
void splash() {
  tft.fillScreen(COL_BG);
  tft.setTextColor(COL_TITLE);
  tft.setTextSize(2);
  tft.setCursor(6, 6);
  tft.println(F("ArduinoLab"));
  tft.drawFastHLine(0, 26, 240, COL_GREY);

  tft.setTextSize(1);
  tft.setTextColor(COL_TEXT);
  tft.setCursor(6, 32);
  tft.print(F("Firmware: "));
  tft.println(F(FW_VERSION));

  tft.setCursor(6, 44);
  tft.print(F("Free RAM: "));
  tft.print(freeRam());
  tft.println(F(" bytes"));

  tft.drawFastHLine(0, 56, 240, COL_GREY);
  tft.setTextColor(COL_WARN);
  tft.setCursor(6, 62);
  tft.println(F("Scanning I2C bus:"));

  Serial.println(F("--- I2C scan ---"));
  int y = 74;
  byte count = 0;
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      String nm = i2cDeviceName(addr);

      Serial.print(F("  Found device 0x"));
      if (addr < 16) Serial.print('0');
      Serial.print(addr, HEX);
      if (nm.length()) { Serial.print(F("  ")); Serial.print(nm); }
      Serial.println();

      tft.setTextColor(COL_OK);
      tft.setCursor(6, y);
      tft.print(F("0x"));
      if (addr < 16) tft.print('0');
      tft.print(addr, HEX);
      tft.setTextColor(COL_TEXT);
      tft.print(F("  "));
      tft.println(nm);

      y += 10;
      count++;
      delay(2);
    }
  }

  if (count == 0) {
    tft.setTextColor(COL_FAIL);
    tft.setCursor(6, y);
    tft.println(F("No devices found!"));
    y += 10;
  }

  tft.drawFastHLine(0, y + 3, 240, COL_GREY);
  tft.setTextColor(COL_TEXT);
  tft.setCursor(6, y + 9);
  tft.print(F("Total found: "));
  tft.println(count);

  Serial.print(F("Total I2C devices: "));
  Serial.println(count);
  Serial.println(F("----------------"));

  delay(5000); // пауза перед переходом в меню
}

// ============================ ТЕСТ TFT =====================================
void testTFT() {
  screenHeader(F("1. TFT ST7789"));
  tft.fillRect(0, 40, 240, 60, ST77XX_RED);
  tft.fillRect(0, 100, 240, 60, ST77XX_GREEN);
  tft.fillRect(0, 160, 240, 60, ST77XX_BLUE);
  printLine(80, F("R"), COL_TEXT);
  printLine(140, F("G"), COL_TEXT);
  printLine(200, F("B"), COL_TEXT);
  delay(1200);

  screenHeader(F("1. TFT ST7789"));
  statusBadge(true, 40);
  printLine(80, F("Init: OK"), COL_TEXT);
  printLine(105, F("240x240, SPI"), COL_TEXT);
  printLine(130, F("Backlight: always on"), COL_TEXT);
  logResult(F("TFT ST7789"), true, F("init and draw completed successfully (backlight not software-controlled)"));
  delay(1200);
}

// ======================= ТЕСТ КОМПАСА (регистры напрямую) ========================
bool i2cPresent(byte addr) {
  Wire.beginTransmission(addr);
  return (Wire.endTransmission() == 0);
}

bool i2cWriteReg(byte addr, byte reg, byte val) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(val);
  return (Wire.endTransmission() == 0);
}

// Определяем реальный чип на плате: многие модули GY-273, подписанные как
// HMC5883L, на деле содержат клон QMC5883L — у него ДРУГОЙ адрес I2C (0x0D)
// и другая раскладка регистров/данных, поэтому поддерживаем оба варианта.
enum CompassChip { COMPASS_NONE, COMPASS_HMC5883L, COMPASS_QMC5883L };
CompassChip compassChip = COMPASS_NONE;

bool compassBegin() {
  if (i2cPresent(I2C_ADDR_HMC5883L)) {
    i2cWriteReg(I2C_ADDR_HMC5883L, 0x00, 0x70); // Config A: 8 усреднений, 15Hz
    i2cWriteReg(I2C_ADDR_HMC5883L, 0x01, 0xA0); // Config B: усиление
    i2cWriteReg(I2C_ADDR_HMC5883L, 0x02, 0x00); // Mode: непрерывный режим
    compassChip = COMPASS_HMC5883L;
    return true;
  }
  if (i2cPresent(I2C_ADDR_QMC5883L)) {
    i2cWriteReg(I2C_ADDR_QMC5883L, 0x0B, 0x01); // SET/RESET period (обязательно)
    i2cWriteReg(I2C_ADDR_QMC5883L, 0x09, 0x1D); // OSR=512,RNG=8G,ODR=200Hz,continuous
    compassChip = COMPASS_QMC5883L;
    return true;
  }
  compassChip = COMPASS_NONE;
  return false;
}

// Порядок байт данных в регистрах разный у чипов: HMC5883L отдаёт X,Z,Y
// (старший байт первым), QMC5883L — X,Y,Z (младший байт первым).
bool compassRead(int16_t &x, int16_t &y, int16_t &z) {
  if (compassChip == COMPASS_HMC5883L) {
    Wire.beginTransmission(I2C_ADDR_HMC5883L);
    Wire.write(0x03);
    if (Wire.endTransmission() != 0) return false;
    Wire.requestFrom((int)I2C_ADDR_HMC5883L, 6);
    if (Wire.available() < 6) return false;
    x = (int16_t)((Wire.read() << 8) | Wire.read());
    z = (int16_t)((Wire.read() << 8) | Wire.read());
    y = (int16_t)((Wire.read() << 8) | Wire.read());
    return true;
  }
  if (compassChip == COMPASS_QMC5883L) {
    Wire.beginTransmission(I2C_ADDR_QMC5883L);
    Wire.write(0x00);
    if (Wire.endTransmission() != 0) return false;
    Wire.requestFrom((int)I2C_ADDR_QMC5883L, 6);
    if (Wire.available() < 6) return false;
    x = (int16_t)(Wire.read() | (Wire.read() << 8));
    y = (int16_t)(Wire.read() | (Wire.read() << 8));
    z = (int16_t)(Wire.read() | (Wire.read() << 8));
    return true;
  }
  return false;
}

void testCompassHMC5883L() {
  screenHeader(F("2. Compass HMC5883L"));

  static bool inited = false;
  static bool initOk = false;
  if (!inited) {
    initOk = compassBegin();
    inited = true;
  }

  if (!initOk) {
    statusBadge(false, 40);
    printLine(80, F("I2C 0x1E/0x0D no reply"), COL_FAIL);
    logResult(F("HMC5883L/QMC5883"), false, F("device not found on I2C bus (neither 0x1E nor 0x0D)"));
    delay(1500);
    return;
  }

  printExitHint();
  waitLadderRelease();
  while (true) {
    int16_t x, y, z;
    bool ok = compassRead(x, y, z);

    statusBadge(ok, 40);
    if (ok) {
      printLine(80, String(F("X: ")) + String((int)x), COL_TEXT);
      printLine(105, String(F("Y: ")) + String((int)y), COL_TEXT);
      printLine(130, String(F("Z: ")) + String((int)z), COL_TEXT);
      float heading = atan2((float)y, (float)x) * 180.0 / PI;
      if (heading < 0) heading += 360.0;
      printLine(160, String(F("Heading: ")) + String(heading, 1) + F(" deg"), COL_WARN);
      printLine(190, compassChip == COMPASS_HMC5883L ? F("Chip: HMC5883L") : F("Chip: QMC5883L"), COL_TEXT, 1);
      logResult(F("HMC5883L/QMC5883"), true,
                String(F("X=")) + String((int)x) + F(" Y=") + String((int)y) +
                F(" Z=") + String((int)z) + F(" heading=") + String(heading, 1));
    } else {
      printLine(80, F("Read error"), COL_FAIL);
      logResult(F("HMC5883L/QMC5883"), false, F("no data after request"));
    }

    if (liveWait(1000)) { skipReturnWait = true; break; } // выход по любой кнопке, иначе обновление через 1с
  }
}

// ============================ ТЕСТ RTC DS3232 ================================
// Флаг остановки генератора (OSF, регистр 0x0F, бит 7) читаем напрямую по I2C —
// не зависит от конкретной RTC-библиотеки. true = было отключение питания/
// разряжена батарейка BT1, время могло сбиться.
bool ds3232OscillatorStopped() {
  Wire.beginTransmission(I2C_ADDR_DS3232);
  Wire.write(0x0F);
  if (Wire.endTransmission() != 0) return false;
  Wire.requestFrom((int)I2C_ADDR_DS3232, 1);
  if (Wire.available() < 1) return false;
  byte status = Wire.read();
  return (status & 0x80) != 0;
}

inline byte bcd2bin(byte val) { return (val & 0x0F) + ((val >> 4) * 10); }

// Читает время/дату из регистров 0x00-0x06 DS3231/DS3232 (стандартная
// раскладка, без всякой библиотеки). Час корректно обрабатывается и в
// 12-, и в 24-часовом режиме (бит6 регистра часов), на случай если модуль
// ранее был настроен в 12-часовой режим каким-то другим устройством.
bool ds3232ReadTime(byte &hour, byte &minute, byte &second, byte &date, byte &month, int &year) {
  Wire.beginTransmission(I2C_ADDR_DS3232);
  Wire.write(0x00);
  if (Wire.endTransmission() != 0) return false;
  Wire.requestFrom((int)I2C_ADDR_DS3232, 7);
  if (Wire.available() < 7) return false;

  second = bcd2bin(Wire.read() & 0x7F);
  minute = bcd2bin(Wire.read() & 0x7F);

  byte hourRaw = Wire.read();
  if (hourRaw & 0x40) { // 12-часовой режим
    bool pm = hourRaw & 0x20;
    hour = bcd2bin(hourRaw & 0x1F);
    if (hour == 12) hour = 0;
    if (pm) hour += 12;
  } else {              // 24-часовой режим
    hour = bcd2bin(hourRaw & 0x3F);
  }

  Wire.read();          // день недели — не используется, но нужно вычитать
  date = bcd2bin(Wire.read() & 0x3F);
  byte monthRaw = Wire.read();
  month = bcd2bin(monthRaw & 0x1F);
  bool century = monthRaw & 0x80;
  year = 2000 + bcd2bin(Wire.read()) + (century ? 100 : 0);
  return true;
}

void testRTC_DS3232() {
  screenHeader(F("3. RTC DS3232"));

  if (!i2cPresent(I2C_ADDR_DS3232)) {
    statusBadge(false, 40);
    printLine(80, F("Not found (0x68)"), COL_FAIL);
    logResult(F("DS3232"), false, F("not found on I2C 0x68"));
    delay(1500);
    return;
  }

  byte hour, minute, second, date, month;
  int year;
  bool ok = ds3232ReadTime(hour, minute, second, date, month, year);
  bool lowBat = ds3232OscillatorStopped();

  statusBadge(ok, 40);
  if (ok) {
    char buf[24];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", hour, minute, second);
    printLine(80, String(buf), COL_TEXT, 3);
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d", year, month, date);
    printLine(115, String(buf), COL_TEXT);
    printLine(150, lowBat ? F("Battery: CHECK") : F("Battery: OK"),
              lowBat ? COL_WARN : COL_OK);
    logResult(F("DS3232"), true, String(buf) + (lowBat ? F(" (BT1 battery low!)") : F("")));
  } else {
    printLine(80, F("Read error"), COL_FAIL);
    logResult(F("DS3232"), false, F("no data after register request"));
  }
  delay(1500);
}

// ========================= ТЕСТ EEPROM AT24C32 ================================
bool eepromWriteByte(uint16_t addr, byte data) {
  Wire.beginTransmission(I2C_ADDR_EEPROM);
  Wire.write((byte)(addr >> 8));
  Wire.write((byte)(addr & 0xFF));
  Wire.write(data);
  bool ok = (Wire.endTransmission() == 0);
  delay(6); // время записи страницы EEPROM
  return ok;
}

int eepromReadByte(uint16_t addr) {
  Wire.beginTransmission(I2C_ADDR_EEPROM);
  Wire.write((byte)(addr >> 8));
  Wire.write((byte)(addr & 0xFF));
  if (Wire.endTransmission() != 0) return -1;
  Wire.requestFrom((int)I2C_ADDR_EEPROM, 1);
  if (Wire.available() < 1) return -1;
  return Wire.read();
}

void testEEPROM_AT24C32() {
  screenHeader(F("4. EEPROM AT24C32"));

  if (!i2cPresent(I2C_ADDR_EEPROM)) {
    statusBadge(false, 40);
    printLine(80, F("No reply on 0x50"), COL_FAIL);
    printLine(105, F("check A0-A2 address"), COL_WARN, 1);
    logResult(F("AT24C32"), false, F("no reply at address 0x50 - check address pins A0..A2"));
    delay(1500);
    return;
  }

  const uint16_t testAddr = 0x0000;
  byte testValue = (byte)(millis() & 0xFF);

  bool wOk = eepromWriteByte(testAddr, testValue);
  int rVal = eepromReadByte(testAddr);
  bool ok = wOk && (rVal == testValue);

  statusBadge(ok, 40);
  printLine(80, String(F("Write: ")) + String(testValue), COL_TEXT);
  printLine(105, String(F("Read: ")) + String(rVal), COL_TEXT);
  logResult(F("AT24C32"), ok, String(F("written=")) + String(testValue) + F(" read=") + String(rVal));
  delay(1500);
}

// ============================ ТЕСТ DHT11 ====================================
void testDHT11() {
  screenHeader(F("5. DHT11"));
  printExitHint();
  waitLadderRelease();
  while (true) {
    int temperature = 0;
    int humidity = 0;
    int result = dhtSensor.readTemperatureHumidity(temperature, humidity);
    bool ok = (result == 0);

    statusBadge(ok, 40);
    if (ok) {
      printLine(80, String(F("Temp: ")) + String(temperature) + F(" C"), COL_TEXT, 3);
      printLine(120, String(F("Humidity: ")) + String(humidity) + F(" %"), COL_TEXT);
      logResult(F("DHT11"), true, String(F("t=")) + String(temperature) + F("C h=") + String(humidity) + F("%"));
    } else {
      String err = dhtSensor.getErrorString(result);
      printLine(80, F("Error:"), COL_FAIL);
      printLine(105, err, COL_FAIL, 1);
      logResult(F("DHT11"), false, err);
    }

    if (liveWait(1000)) { skipReturnWait = true; break; } // выход по любой кнопке, иначе обновление через 1с
  }
}

// ============================ ТЕСТ LM35 =====================================
void testLM35() {
  screenHeader(F("6. LM35 (analog)"));
  printExitHint();
  waitLadderRelease();
  while (true) {
    int raw = analogRead(PIN_LM35);
    float voltage = raw * (5.0 / 1023.0);
    float tempC = voltage * 100.0; // LM35: 10mV/°C

    statusBadge(true, 40);
    printLine(80, String(F("ADC: ")) + String(raw), COL_TEXT);
    printLine(105, String(F("U: ")) + String(voltage, 3) + F(" V"), COL_TEXT);
    printLine(140, String(F("T: ")) + String(tempC, 1) + F(" C"), COL_WARN, 3);
    logResult(F("LM35"), true, String(F("ADC=")) + String(raw) + F(" T=") + String(tempC, 1) + F("C"));

    if (liveWait(1000)) { skipReturnWait = true; break; }
  }
}

// ============================ ТЕСТ LDR (GL5549) ===============================
void testLDR() {
  screenHeader(F("7. Photoresistor LDR"));
  printExitHint();
  waitLadderRelease();
  while (true) {
    int raw = analogRead(PIN_LDR);
    float voltage = raw * (5.0 / 1023.0);

    statusBadge(true, 40);
    printLine(80, String(F("ADC: ")) + String(raw), COL_TEXT, 3);
    printLine(115, String(F("U: ")) + String(voltage, 2) + F(" V"), COL_TEXT);
    String level = raw > 700 ? F("Dark") : (raw > 300 ? F("Medium") : F("Bright"));
    printLine(150, String(F("Level: ")) + level, COL_WARN);
    logResult(F("LDR (GL5549)"), true, String(F("ADC=")) + String(raw) + F(" (") + level + F(")"));

    if (liveWait(1000)) { skipReturnWait = true; break; }
  }
}

// ============================ ТЕСТ ПЕРЕМЕННОГО РЕЗИСТОРА (A0) ===================
void testSpareA0() {
  screenHeader(F("8. Variable Resistor"));
  printExitHint();
  waitLadderRelease();
  while (true) {
    int raw = analogRead(PIN_SPARE_A0);
    float voltage = raw * (5.0 / 1023.0);

    statusBadge(true, 40);
    printLine(80, String(F("ADC: ")) + String(raw), COL_TEXT, 3);
    printLine(115, String(F("U: ")) + String(voltage, 2) + F(" V"), COL_TEXT);
    logResult(F("A0 (variable R)"), true, String(F("ADC=")) + String(raw) + F(" U=") + String(voltage, 2) + F("V"));

    if (liveWait(1000)) { skipReturnWait = true; break; }
  }
}

// ======================== ТЕСТ КНОПОК D2 / D3 ==================================
void testDigitalButtons() {
  screenHeader(F("9. Buttons D2/D3"));
  printExitHint();
  waitLadderRelease();

  bool b2 = false, b3 = false; // начальные значения не важны — см. флаг first
  bool first = true;

  while (true) {
    bool nb2 = (digitalRead(PIN_BTN_D2) == LOW); // SW3
    bool nb3 = (digitalRead(PIN_BTN_D3) == LOW); // SW4

    if (first || nb2 != b2 || nb3 != b3) {
      b2 = nb2; b3 = nb3; first = false;
      printLine(50, String(F("SW3 (D2): ")) + (b2 ? F("PRESSED ") : F("released ")),
                b2 ? COL_OK : COL_TEXT);
      printLine(90, String(F("SW4 (D3): ")) + (b3 ? F("PRESSED ") : F("released ")),
                b3 ? COL_OK : COL_TEXT);
      logResult(F("SW3/D2"), true, b2 ? F("pressed") : F("released"));
      logResult(F("SW4/D3"), true, b3 ? F("pressed") : F("released"));
    }

    if (liveWait(1000)) { skipReturnWait = true; break; }
  }
}

// ============================ ТЕСТ ДАЛЬНОМЕРА VL53L0X ===========================
// Библиотека Pololu VL53L0X (github.com/pololu/vl53l0x-arduino) — не зависит
// от Adafruit_BusIO, гораздо компактнее Adafruit_VL53L0X.
void testDistanceVL53L0X() {
  screenHeader(F("10. Distance VL53L0X"));

  static bool inited = false;
  static bool initOk = false;
  if (!inited) {
    distanceSensor.setTimeout(500);
    initOk = distanceSensor.init();
    inited = true;
  }

  if (!initOk) {
    statusBadge(false, 40);
    printLine(80, F("Init FAIL (0x29)"), COL_FAIL);
    logResult(F("VL53L0X"), false, F("init failed / not found on I2C 0x29"));
    delay(1500);
    return;
  }

  printExitHint();
  waitLadderRelease();
  while (true) {
    uint16_t mm = distanceSensor.readRangeSingleMillimeters();
    bool timeout = distanceSensor.timeoutOccurred();

    statusBadge(!timeout, 40);
    if (!timeout) {
      printLine(80, F("Distance:"), COL_TEXT);
      printLine(105, String(mm) + F(" mm"), COL_WARN, 3);
      logResult(F("VL53L0X"), true, String(mm) + F(" mm"));
    } else {
      printLine(80, F("Timeout / out of range"), COL_FAIL, 1);
      logResult(F("VL53L0X"), false, F("sensor timeout"));
    }

    if (liveWait(1000)) { skipReturnWait = true; break; } // выход по любой кнопке, иначе обновление через 1с
  }
}

// ============================ ТЕСТ БУЗЗЕРА ===================================
void testBuzzer() {
  screenHeader(F("11. Buzzer (SG1)"));
  printLine(80, F("Signal..."), COL_WARN);

  for (int i = 0; i < 3; i++) {
    digitalWrite(PIN_BUZZER, BUZZER_ON);
    delay(150);
    digitalWrite(PIN_BUZZER, BUZZER_OFF);
    delay(150);
  }

  statusBadge(true, 110);
  printLine(150, F("3 short beeps"), COL_TEXT);
  logResult(F("Buzzer/Q1"), true, F("3 short beeps issued on D5"));
  delay(1000);
}

// ============================ ТЕСТ HC-12 =====================================
// SET (D6) управляется программно через перемычку H1 (норма: 3-5 и 4-6):
//   SET = HIGH -> обычный режим передачи данных (транспарентный UART-мост)
//   SET = LOW  -> режим AT-команд: модуль сам отвечает "OK" на команду "AT",
//                 это позволяет проверить модуль БЕЗ второго HC-12 на другом конце.
bool hc12SendAT(const __FlashStringHelper* cmd, String &response, unsigned long timeoutMs = 300) {
  while (hc12.available()) hc12.read(); // очистка буфера
  hc12.print(cmd);
  hc12.print(F("\r\n"));
  response = "";
  unsigned long t0 = millis();
  while (millis() - t0 < timeoutMs) {
    while (hc12.available()) response += (char)hc12.read();
  }
  response.trim();
  return response.length() > 0;
}

void testHC12() {
  screenHeader(F("12. HC-12 (radio)"));
  printLine(45, F("TX:D7 RX:D8 SET:D6"), COL_TEXT, 1);

  // --- шаг 1: режим AT-команд (SET=LOW), проверяем ответ самого модуля ---
  digitalWrite(PIN_HC12_SET, HC12_MODE_AT);
  delay(60); // модулю нужно время войти в режим команд

  String resp;
  bool atOk = hc12SendAT(F("AT"), resp);

  printLine(65, F("AT mode (SET=0):"), COL_WARN, 1);
  if (atOk) {
    printLine(85, String(F("Reply: ")) + resp, COL_OK, 1);
  } else {
    printLine(85, F("No reply from module"), COL_FAIL, 1);
  }

  // на всякий случай спросим версию модуля
  String verResp;
  bool verOk = hc12SendAT(F("AT+V"), verResp);
  if (verOk) printLine(105, String(F("Version: ")) + verResp, COL_TEXT, 1);

  // --- шаг 2: возвращаем нормальный режим передачи (SET=HIGH) ---
  digitalWrite(PIN_HC12_SET, HC12_MODE_NORMAL);
  delay(60);

  hc12.println(F("ArduinoLab test"));
  delay(100);
  bool gotEcho = false;
  String echo = "";
  unsigned long t0 = millis();
  while (millis() - t0 < 300) {
    while (hc12.available()) { echo += (char)hc12.read(); gotEcho = true; }
  }

  bool overallOk = atOk; // главный критерий исправности — ответ модуля в AT-режиме
  statusBadge(overallOk, 130);
  printLine(160, F("Normal mode (SET=1):"), COL_TEXT, 1);
  printLine(180, F("packet sent"), COL_TEXT, 1);
  printLine(200, gotEcho ? (String(F("reply: ")) + echo) : String(F("no reply (normal)")), COL_TEXT, 1);

  logResult(F("HC-12"), overallOk,
            String(F("AT mode: ")) + (atOk ? (String(F("responds (")) + resp + F(")")) : String(F("no response"))) +
            (verOk ? (String(F("; version: ")) + verResp) : String(F(""))) +
            F("; normal mode: packet sent") + (gotEcho ? (String(F(", reply: ")) + echo) : String(F(", no reply (normal without a second module)"))));
  delay(1800);
}
