#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

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
// Разрешение экрана
#define W 240
#define H 240
// Размер куба
float cubeSize = 60;
// Углы текущие
float angleX = 0;
float angleY = 0;
float angleZ = 0;
// Вершины 3D-куба
float vertices[8][3] = {
  {-1, -1, -1}, {1, -1, -1},
  {1,  1, -1}, {-1,  1, -1},
  {-1, -1,  1}, {1, -1,  1},
  {1,  1,  1}, {-1,  1,  1}
};

// Ребра куба (пары индексов вершин)
int edges[12][2] = {
  {0,1},{1,2},{2,3},{3,0},
  {4,5},{5,6},{6,7},{7,4},
  {0,4},{1,5},{2,6},{3,7}
};

// Спроецированные 2D точки
int proj[8][2];

void setup() {
 // Инициализация дисплея
 #if (HARDWARE == 120)
  tft.init(240, 240, SPI_MODE3);// Новая версия
#else  
  tft.init(240, 240, SPI_MODE2);// Init ST7789 240x240 Параметр SPI_MODE2 ОБЯЗАТЕЛЕН! (без него дисплей не рабоатет!) для версии 110
#endif
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);  
  tft.cp437(true);

}

// ==== 3D-ВРАЩЕНИЕ ====
void rotate(float &x, float &y, float &z) {
  float sinX = sin(angleX), cosX = cos(angleX);
  float sinY = sin(angleY), cosY = cos(angleY);
  float sinZ = sin(angleZ), cosZ = cos(angleZ);

  // Вращение X
  float y1 = y * cosX - z * sinX;
  float z1 = y * sinX + z * cosX;
  y = y1; z = z1;

  // Вращение Y
  float x1 = x * cosY + z * sinY;
  float z2 = -x * sinY + z * cosY;
  x = x1; z = z2;

  // Вращение Z
  float x2 = x * cosZ - y * sinZ;
  float y2 = x * sinZ + y * cosZ;
  x = x2; y = y2;
}

// ==== ПРОЕКЦИЯ ====
void project(float x, float y, float z, int &px, int &py) {
  float distance = 3.0;
  float scale = cubeSize;

  float factor = scale / (z + distance);

  px = (int)(x * factor + W / 2);
  py = (int)(y * factor + H / 2);
}

void loop() {
  tft.fillScreen(ST77XX_BLACK);

  // Преобразовать вершины
  for (int i = 0; i < 8; i++) {
    float x = vertices[i][0];
    float y = vertices[i][1];
    float z = vertices[i][2];

    rotate(x, y, z);
    project(x, y, z, proj[i][0], proj[i][1]);
  }

  // Нарисовать края
  for (int i = 0; i < 12; i++) {
    int a = edges[i][0];
    int b = edges[i][1];

    tft.drawLine(
      proj[a][0], proj[a][1],
      proj[b][0], proj[b][1],
      ST77XX_GREEN
    );
  }

  // Обновить ротацию
  angleX += 0.03;
  angleY += 0.02;
  angleZ += 0.04;

 // delay(30); // Задержа между циклами
}
