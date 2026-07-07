/*
Программа демонстрирует, как реализовать функцию компаса. 
Во время работы программы, пожалуйста, свободно вращайте микросхему 
QMC5883 для выполнения калибровки.
 */

#include <Wire.h>
#include <DFRobot_QMC5883.h> // Библиотека компаса

DFRobot_QMC5883 compass;

void setup()
{
  Serial.begin(115200);
  while (!compass.begin())
  {
    Serial.println("Не удалось найти датчик QMC5883, проверьте соединение!");
    delay(500);
  }
    if(compass.isHMC()){
        Serial.println("Инициализация HMC5883");
        compass.setRange(HMC5883L_RANGE_1_3GA);
        compass.setMeasurementMode(HMC5883L_CONTINOUS);
        compass.setDataRate(HMC5883L_DATARATE_15HZ);
        compass.setSamples(HMC5883L_SAMPLES_8);
    }
   else if(compass.isQMC()){
        Serial.println("Инициализация QMC5883");
        compass.setRange(QMC5883_RANGE_2GA);
        compass.setMeasurementMode(QMC5883_CONTINOUS); 
        compass.setDataRate(QMC5883_DATARATE_50HZ);
        compass.setSamples(QMC5883_SAMPLES_8);
   }
  }
void loop()
{
  Vector norm = compass.readNormalize();
  // Вычисление направления (радианы)
  float heading = atan2(norm.YAxis, norm.XAxis);

// Установите угол склонения для вашего местоположения и зафиксируйте курс
// Вы можете узнать свое склонение на сайте: http://magnetic-declination.com/
// (+) Положительное или (-) Отрицательное
// Для Москвы угол склонения 12'7 E (положительный)
// Формула: (град + (мин / 60,0)) / (180 / M_PI);
  float declinationAngle = (12.0 + (7.0 / 60.0)) / (180 / PI);
  heading += declinationAngle;

  // Корректировкуа направления < 0 до > 360
  if (heading < 0){
    heading += 2 * PI;
  }

  if (heading > 2 * PI){
    heading -= 2 * PI;
  }

  // Перевод в градусы
  float headingDegrees = heading * 180/M_PI; 

  // Вывод в консоль
  Serial.print(" Heading = ");
  Serial.print(heading);
  Serial.print(" Degress = ");
  Serial.print(headingDegrees);
  Serial.println();

  delay(100);
}
