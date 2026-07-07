/**************************************************************************
  Программа тестирования лабораторного макета
 **************************************************************************/
#include <SPI.h>
#include <Wire.h>  // подключим стандартную библиотеку I2C
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
// Внимание! Существует два типа компасов оригинальны (0x1e) и китайский аналог (0x0d) в программе это определяеться и работает с обоими
#include <DFRobot_QMC5883.h> // Библиотека поддержтвает оба типа компаса
#include <VL53L0X.h>
#include <DHT11.h>
#include <SoftwareSerial.h>

#define HARDWARE  120  // Номер версии платы (есть еще 110 версия)
#define VERSION   1.28 // Номер версии ПО
#define Hi        24   // Высота строки в пикселях
// Железо
#define KEY_D2   2
#define KEY_D3   3 
#define BEEP     5    // Пищалка 
#define LED1    13    // Светодиод D13 - дублируется с SCL !!
#define LED2    12    // Светодиод D12 - дублируется с MISO!!
#define SET     6     // Низкий уровень переводит в режим команд HC-12 высокий передача данных

// Определение ног дисплея в зависимости от версии платы  
// Дисплей не запускается без управления сброса RES
#if (HARDWARE == 120) 
  #define TFT_CS        0  // Не используется - в дисплее его нет!
  #define TFT_RST       10 // Дисплей не запускается без управления сброса RES
  #define TFT_DC        9  // Команды
#else
  #define TFT_CS        0  // Не используется - в дисплее его нет!
  #define TFT_RST       10 // Дисплей не запускается без управления сброса RES
  #define TFT_DC        9  // Команды
#endif  
// For 1.14", 1.3", 1.54", 1.69", and 2.0" TFT with ST7789:
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);  // Объявление дисплея

// HMC5883L/QMC5883L Компас
DFRobot_QMC5883 compass;                                        // Компас
VL53L0X sensor;                                                 // Дальномер
DHT11 dht11(4);                                                 // Температура/влажность
//RF24 ModuleToTest(8,7); //для Uno другие CE_PIN,SS_PIN:
SoftwareSerial HC12Serial(8, 7); // softRX, softTX

bool _VL53L0X=true;
bool _HMC5883L=true;
const uint64_t pipe = 'ABCDEFG';

void setup(void) {
// Инициалаизация пинов     
pinMode(KEY_D2, INPUT); 
pinMode(KEY_D3, INPUT); 
pinMode(BEEP,   OUTPUT);
//pinMode(LED1,   OUTPUT);   // Дисплей не будет рабоать
pinMode(LED2,   OUTPUT);  
pinMode(SET,   OUTPUT);  
digitalWrite(LED2, LOW);  // Выключить светодиод  
digitalWrite(BEEP, HIGH); // Выключить пищалку
digitalWrite(SET, HIGH);  // HC-12 режим передачи данных

  Serial.begin(9600);
  Serial.println(F("Hello! ArduinoLab Test"));


// Инициалаизация дисплея
#if (HARDWARE == 120)
  tft.init(240, 240, SPI_MODE3);           // Новая версия
#else  
  tft.init(240, 240, SPI_MODE2);           // Init ST7789 240x240 Параметр SPI_MODE2 ОБЯЗАТЕЛЕН! (без него дисплей не рабоатет!) для версии 110
#endif
  tft.setRotation(1);
  
// Стартовый экран  
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(3);
  tft.setCursor(30,0);
  tft.println("ArduinoLab\n");  
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_BLUE);
  tft.print("Version: ");   tft.print(VERSION);  
 
// Сканирование i2c шины
Wire.begin(); // инициализация I2C
 tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);
 byte error, address;
 int nDevices;
 nDevices = 0;
    Serial.println("Scanning I2C ...");
    tft.println("\n\nScan I2C bus..."); 
    for(address = 8; address < 127; address++ ){
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        if (error == 0){
              Serial.print("Device found at address: 0x");
              tft.print("Device found 0x");
            if (address<16) Serial.print("0");
            Serial.println(address,HEX);
            tft.println(address,HEX);
            nDevices++;
        }
        else if (error==4) {
            Serial.print("Unknow error at address 0x");
            tft.println("Unknow error at 0x");
            if (address<16)
                Serial.print("0");
            Serial.println(address,HEX);
            tft.println(address,HEX);
        } 
    }
    if (nDevices == 0){
        Serial.println("No I2C devices found\n");
        tft.println("No I2C devices found");
    }
    else{
        Serial.println("done\n"); 
        tft.println("done");
    }

  if (!compass.begin()) // попытка инициализировать компас
  {
    Serial.println("Could not find a valid HMC5883L/QMC5883 sensor, check wiring!");
    tft.println("Could not find HMC5883L/QMC5883 ");
    _HMC5883L=false;
  }    
  else // Компас найден
  { tft.print("\nFound: ");
    if(compass.isHMC()) tft.println("HMC5883L"); 
    else                tft.println("QMC5883L");
  }
// Пауза
 for(int i=0;i<2;i++){
    digitalWrite(BEEP, LOW);  // Включить пищалку
    digitalWrite(LED2, HIGH);  // Включить светодиод
    delay(200); 
    digitalWrite(LED2, LOW);  // Выключить светодиод  
    digitalWrite(BEEP, HIGH); // Выключить пищалку
    delay(300); 
  }   
  
// Пауза со шкалой 
 for(int i=0;i<100;i++)  // пауза для чтения инфы
 {
  tft.fillRect(18+2*i,220, 2, 6, ST77XX_YELLOW);
delay(20);
 }
  // Основной экран
  // OR use this initializer (uncomment) if using a 1.3" or 1.54" 240x240 TFT:
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(3);
  tft.setCursor(0,0);
  
  tft.println("Key D2:");  
  tft.println("Key D3:");  

  tft.println("Resistor:"); 
  tft.println("Photo:"); 
  tft.println("LM35:");  
  tft.println("AKey:");  

 // в зависимости от чипа настройка компаса и вывод на экран его типа
   if(compass.isHMC()){
        tft.println("HMC5883L:");
        Serial.println("Initialize HMC5883");
        compass.setRange(HMC5883L_RANGE_1_3GA);
        compass.setMeasurementMode(HMC5883L_CONTINOUS);
        compass.setDataRate(HMC5883L_DATARATE_15HZ);
        compass.setSamples(HMC5883L_SAMPLES_8);
    }
   else if(compass.isQMC()){
        tft.println("QMC5883L:");
        Serial.println("Initialize QMC5883");
        compass.setRange(QMC5883_RANGE_2GA);
        compass.setMeasurementMode(QMC5883_CONTINOUS); 
        compass.setDataRate(QMC5883_DATARATE_50HZ);
        compass.setSamples(QMC5883_SAMPLES_8);
   }
  else tft.println("Compass");
  tft.println("VL53L0X:");
  tft.println("DHT11:");  
  tft.println("HC12:"); 
 

// Инициализация VL53L0X
 sensor.setTimeout(500);
 if (!sensor.init()) // проверка работы датчика расстояния
  {  
    Serial.println("Failed to boot VL53L0X");
    _VL53L0X=false;
  } else Serial.println("Init VL53L0X Ok");
   sensor.startContinuous();// start continuous ranging

// Инициализация и тестирование радиомодуля HC-12
// set the data rate for the SoftwareSerial port
  HC12Serial.begin(9600);
  Serial.println("HC-12 test:");
  digitalWrite(SET, LOW);  // HC-12 режим комманд
  delay(50);

/*
while(1){
	 if (HC12Serial.available()) {
    Serial.write(HC12Serial.read());
  }
  if (Serial.available()) {
    HC12Serial.write(Serial.read());
  }
}
*/  
  HC12Serial.println("AT"); Serial.println("AT");
  delay(50);
  while(HC12Serial.available()) Serial.write(HC12Serial.read());  // Вывести ответ в консоль
 
  HC12Serial.print("AT+V"); Serial.println("AT+V");
  delay(50);
  while(HC12Serial.available()) Serial.write(HC12Serial.read());  // Вывести ответ в консоль

  HC12Serial.print("AT+RX"); Serial.println("AT+RX");
  delay(50);
  while(HC12Serial.available()){ Serial.write(HC12Serial.read()); delay(4); } // Длинный ответ с задержками
  
 digitalWrite(SET, HIGH);  // HC-12 режим данных

analogReference(DEFAULT);
}
// цикл вывода показаний на экран
void loop() {
tft.setTextColor(ST77XX_RED);
// Кнопка D2	
tft.setCursor(140,0); 
tft.fillRect(140, 0*Hi, 80, Hi+1, ST77XX_BLACK);
if(digitalRead(KEY_D2)) tft.println("Off"); else  tft.println("On");

// Кнопка D3	
tft.setCursor(140,1*Hi); 
tft.fillRect(140, 1*Hi, 80, Hi+1, ST77XX_BLACK);
if(digitalRead(KEY_D3)) tft.println("Off"); else  tft.println("On");

// Чтение резистора
tft.setCursor(160,2*Hi); 
tft.fillRect(160, 2*Hi, 80, Hi+1, ST77XX_BLACK);
tft.println(analogRead(A0));

// Чтение фотрезистора
tft.setCursor(160,3*Hi); 
tft.fillRect(160, 3*Hi, 80, Hi+1, ST77XX_BLACK);
tft.println(analogRead(A1));

// Чтение LM35
tft.setCursor(140,4*Hi); 
tft.fillRect(140, 4*Hi, 100, Hi+1, ST77XX_BLACK);
int sum=0;
sum=analogRead(A2);delay(5);
sum=sum+analogRead(A2);delay(5);
sum=sum+analogRead(A2);delay(5);
sum=sum+analogRead(A2);
tft.println((125.0*sum/4.0)/256.0);
//tft.println((125*analogRead(A2))/256.0); //(125*val)>>8

// Чтение аналоговой кнопки
tft.setCursor(160,5*Hi); 
tft.fillRect(160, 5*Hi, 80, Hi+1, ST77XX_BLACK);
uint16_t key=analogRead(A3);
if(key>20){Serial.print("Key:"); Serial.println(key);} // вывод в консоль нажатой кнопки
if (key < 200) tft.println("none"); 
else if (key < 400) tft.println("Down"); 
else if (key < 700) tft.println("Ok"); 
else tft.println("Up");


// Чтение HMC5883L/
  tft.setCursor(160,6*Hi); 
  tft.fillRect(160, 6*Hi, 80, Hi+1, ST77XX_BLACK);
  if(!_HMC5883L )  tft.println("Fail"); else 
  {
  Vector norm = compass.readNormalize();
  float heading = atan2(norm.YAxis, norm.XAxis); // Calculate heading

  // Set declination angle on your location and fix heading
  // You can find your declination on: http://magnetic-declination.com/
  // (+) Positive or (-) for negative
  // For Bytom / Poland declination angle is 4'26E (positive)
  // Formula: (deg + (min / 60.0)) / (180 / M_PI);
  float declinationAngle = (4.0 + (26.0 / 60.0)) / (180 / PI);
  heading += declinationAngle;
 // Correct for heading < 0deg and heading > 360deg
  if (heading < 0)     { heading += 2 * PI; }
  if (heading > 2 * PI){ heading -= 2 * PI; }
  // Convert to degrees
  int16_t headingDegrees = heading * 180/M_PI; 
  tft.println(headingDegrees);
  }
  
// Чтение VL53L
tft.setCursor(160,7*Hi); 
tft.fillRect(160, 7*Hi, 80, Hi+1, ST77XX_BLACK);
if (!_VL53L0X) tft.println("Fail"); else
tft.println(sensor.readRangeContinuousMillimeters());
// if (lox.isRangeComplete()) tft.println(lox.readRange());

// Чтение DHT11
int temperature = 0;
int humidity = 0;
// Attempt to read the temperature and humidity values from the DHT11 sensor.
int result = dht11.readTemperatureHumidity(temperature, humidity);
tft.setCursor(110,8*Hi); 
tft.fillRect(110, 8*Hi, 130, Hi+1, ST77XX_BLACK);
tft.print(temperature);tft.print("/");tft.print(humidity);


// Чтение HC-12
uint8_t  Buf_32bytes[32];
uint8_t i;
  digitalWrite(SET, LOW);  // HC-12 режим комманд
  delay(40);
  HC12Serial.println("AT+RC"); // Прочитать номер канала передачи
  delay(50);
  i=0;
  while(HC12Serial.available()) {Buf_32bytes[i]=HC12Serial.read();i++; if(i>=31) break; } 
  Buf_32bytes[i]=0;
  tft.setCursor(90,9*Hi); 
  tft.fillRect(90, 9*Hi, 150, Hi+1, ST77XX_BLACK);
  if(i>0) tft.print((char*)Buf_32bytes); else tft.print("Error");
  digitalWrite(SET, HIGH);  // HC-12 режим данных

 delay(400);
}
