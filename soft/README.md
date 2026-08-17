# soft — firmware and examples for ArduinoLab

This folder contains Arduino sketches for the ArduinoLab board and the
third-party libraries needed to compile them.

## Structure

- **`ArduinoLabTest/`** — the original board test sketch (v1.28, no on-screen
  menu): initializes every device on the board (display, compass, distance
  sensor, DHT11, HC-12) and cycles through them.
- **`examples/`** — 19 numbered, standalone example sketches (see table below),
  each focused on one device or feature. Good starting points if you only want
  to understand/reuse the code for a single sensor rather than the whole
  diagnostic firmware.
- **`lib/`** — third-party Arduino libraries required by the sketches above
  (zipped; install via *Sketch → Include Library → Add .ZIP Library…*, or
  unpack into your `Arduino/libraries` folder).

> The final example, **`19_ArduinoLab_Diagnostics`**, additionally requires a
> custom `MiniTFT` display library (`#include <MiniTFT.h>`) that is **not**
> included in `lib/` yet — see the note in the repository root `README.md`.

## Examples

| # | Sketch | What it demonstrates |
|---|---|---|
| 01 | `01_DemoTFT` | Basic ST7789 display test using `Adafruit_GFX` + `Adafruit_ST7789` (shapes, text). |
| 02 | `02_DemoTFT` | Extended display demo, adds the on-board piezo buzzer (`BEEP`, pin D5). |
| 03 | `03_DemoTFT` | The most extensive display demo of the three — a broader tour of `Adafruit_GFX` drawing primitives. |
| 04 | `04_3D_cube` | Real-time rotating wireframe 3D cube rendered on the TFT — a simple 3D-graphics/math demo. |
| 05 | `05_QMC5883_compass` | Reading the on-board compass with `DFRobot_QMC5883` (auto-detects HMC5883L vs. QMC5883L); rotate the board to calibrate while it runs. |
| 06 | `06_calibration_QMC5883` | Magnetometer calibration using direct I2C register access (no library) — computing min/max/offset for X/Y/Z. Written with ESP32-style pin numbers as a reference; adapt the I2C pins for Arduino Uno. |
| 07 | `07_QMC5883_TFT` | Combines the compass reading with a live heading readout on the TFT display. |
| 08 | `08_Rus_TFT` | TFT text output demo using a Cyrillic-capable fork of the ST7789 library (`Rus_st7789`, see `lib/`) — for displaying Russian text. |
| 09 | `09_TFT_DHT11` | Reads the on-board DHT11 temperature/humidity sensor and displays the readings on the TFT. |
| 10 | `10_TFT_LM35` | A simple thermometer: reads the on-board LM35 analog sensor and shows the temperature on the TFT, with a buzzer pin defined. |
| 11 | `11_chart_vl53l0x` | VL53L0X distance sensor (Pololu library) with readings plotted as a live chart on the TFT. |
| 12 | `12_TFT_vl53l0x` | VL53L0X distance sensor (Pololu library) with a simple numeric readout on the TFT. |
| 13 | `13_chart_vl53l0x` | Same idea as #11 (live chart), but using the `Adafruit_VL53L0X` library instead of Pololu's. |
| 14 | `14_popolu_vl53l0x` | Minimal single-shot ranging example for the VL53L0X using only the Pololu library — no display, Serial output only. |
| 15 | `15_chart_vl53l0x_led` | Distance chart on the TFT plus LED indication reacting to the measured distance. |
| 16 | `16_MPU6050_raw` | Raw accelerometer/gyroscope data from an **external** MPU6050 module (connect via the board's spare I2C header) using the ElectronicCats MPU6050 library. |
| 17 | `17_MPU6050_Zero` | MPU6050 offset calibration ("IMU Zero") — computes accelerometer/gyroscope bias so the external module reads correctly at rest. |
| 18 | `18_Tenzo` | Load-cell weighing example: an **external** HX711 amplifier + load cell, plus the on-board DHT11 and TFT display. |
| 19 | `19_ArduinoLab_Diagnostics` | **The full diagnostic firmware.** An on-screen menu (Up/OK/Down buttons) with a selectable, live-updating test for every on-board device, an I2C bus scanner, and free-RAM/firmware-version display at startup. Requires the separate `MiniTFT` library — see note above. |

## Libraries (`lib/`)

| Library | Used for |
|---|---|
| `Adafruit-GFX-Library.zip` | Core graphics primitives (used together with `Adafruit-ST7789-Library`) |
| `Adafruit-ST7789-Library.zip` | ST7789 display driver |
| `Rus_st7789.zip` | Cyrillic-capable fork of the ST7789 library, used by `08_Rus_TFT` |
| `DFRobot_QMC5883.zip` | Compass driver (auto-detects HMC5883L / QMC5883L) |
| `DHT11_!!.zip` | DHT11 temperature/humidity sensor driver |
| `DS3231.ZIP` | DS3231/DS3232 real-time clock driver |
| `HX711.zip` | HX711 load-cell amplifier driver, used by `18_Tenzo` |
| `vl53l0x-arduino.zip` | Pololu's VL53L0X distance sensor library |
| `SoftwareSerial.zip` | Software UART, used for the HC-12 radio module |

---

# soft — прошивки и примеры для ArduinoLab

В этой папке лежат скетчи Arduino для платы ArduinoLab и сторонние
библиотеки, необходимые для их сборки.

## Структура

- **`ArduinoLabTest/`** — исходный тестовый скетч платы (v1.28, без меню на
  экране): инициализирует все устройства платы (экран, компас, дальномер,
  DHT11, HC-12) и последовательно опрашивает их по кругу.
- **`examples/`** — 19 пронумерованных самостоятельных примеров (см. таблицу
  ниже), каждый посвящён одному устройству или функции. Хорошая отправная
  точка, если нужен код только под один датчик, а не вся диагностическая
  прошивка целиком.
- **`lib/`** — сторонние Arduino-библиотеки, необходимые для скетчей выше
  (в архивах; ставятся через *Скетч → Подключить библиотеку → Добавить .ZIP
  библиотеку…*, либо распаковкой в папку `Arduino/libraries`).

> Последний пример, **`19_ArduinoLab_Diagnostics`**, дополнительно требует
> собственную библиотеку экрана `MiniTFT` (`#include <MiniTFT.h>`), которой
> **пока нет** в `lib/` — см. примечание в корневом `README.md` репозитория.

## Примеры

| № | Скетч | Что демонстрирует |
|---|---|---|
| 01 | `01_DemoTFT` | Базовый тест экрана ST7789 на `Adafruit_GFX` + `Adafruit_ST7789` (фигуры, текст). |
| 02 | `02_DemoTFT` | Расширенное демо экрана, добавлен пьезозуммер платы (`BEEP`, пин D5). |
| 03 | `03_DemoTFT` | Самое подробное из трёх демо экрана — более широкий обзор примитивов рисования `Adafruit_GFX`. |
| 04 | `04_3D_cube` | Вращающийся в реальном времени каркасный 3D-куб на экране — простое демо 3D-графики/математики. |
| 05 | `05_QMC5883_compass` | Чтение установленного на плате компаса через `DFRobot_QMC5883` (автоопределение HMC5883L/QMC5883L); во время работы поворачивайте плату для калибровки. |
| 06 | `06_calibration_QMC5883` | Калибровка магнитометра напрямую через регистры I2C (без библиотеки) — вычисление min/max/offset по осям X/Y/Z. Написан с пинами в стиле ESP32 как справочный пример; для Arduino Uno пины I2C нужно адаптировать. |
| 07 | `07_QMC5883_TFT` | Показания компаса вместе с «живым» азимутом на экране. |
| 08 | `08_Rus_TFT` | Демо вывода текста на экран с использованием форка библиотеки ST7789 с поддержкой кириллицы (`Rus_st7789`, см. `lib/`) — для вывода русского текста. |
| 09 | `09_TFT_DHT11` | Чтение установленного на плате датчика DHT11 (температура/влажность) с выводом на экран. |
| 10 | `10_TFT_LM35` | Простой термометр: чтение установленного на плате аналогового датчика LM35 с выводом температуры на экран, определён пин зуммера. |
| 11 | `11_chart_vl53l0x` | Дальномер VL53L0X (библиотека Pololu), показания рисуются как «живой» график на экране. |
| 12 | `12_TFT_vl53l0x` | Дальномер VL53L0X (библиотека Pololu) с простым числовым выводом на экран. |
| 13 | `13_chart_vl53l0x` | То же самое, что №11 (живой график), но на библиотеке `Adafruit_VL53L0X` вместо Pololu. |
| 14 | `14_popolu_vl53l0x` | Минимальный пример однократного измерения VL53L0X только на библиотеке Pololu — без экрана, вывод только в Serial. |
| 15 | `15_chart_vl53l0x_led` | График расстояния на экране плюс индикация светодиодом в зависимости от измеренного расстояния. |
| 16 | `16_MPU6050_raw` | Сырые данные акселерометра/гироскопа с **внешнего** модуля MPU6050 (подключается через запасной разъём I2C платы), библиотека ElectronicCats MPU6050. |
| 17 | `17_MPU6050_Zero` | Калибровка смещений MPU6050 ("IMU Zero") — вычисляет смещения акселерометра/гироскопа, чтобы внешний модуль корректно показывал ноль в состоянии покоя. |
| 18 | `18_Tenzo` | Пример взвешивания на тензодатчике: **внешний** усилитель HX711 + тензодатчик, плюс установленные на плате DHT11 и экран. |
| 19 | `19_ArduinoLab_Diagnostics` | **Полная диагностическая прошивка.** Меню на экране (кнопки Up/OK/Down) с выбираемым, обновляющимся «вживую» тестом для каждого устройства платы, сканер шины I2C, вывод свободной RAM/версии прошивки при старте. Требует отдельную библиотеку `MiniTFT` — см. примечание выше. |

## Библиотеки (`lib/`)

| Библиотека | Для чего используется |
|---|---|
| `Adafruit-GFX-Library.zip` | Базовые графические примитивы (используется вместе с `Adafruit-ST7789-Library`) |
| `Adafruit-ST7789-Library.zip` | Драйвер экрана ST7789 |
| `Rus_st7789.zip` | Форк библиотеки ST7789 с поддержкой кириллицы, используется в `08_Rus_TFT` |
| `DFRobot_QMC5883.zip` | Драйвер компаса (автоопределение HMC5883L / QMC5883L) |
| `DHT11_!!.zip` | Драйвер датчика температуры/влажности DHT11 |
| `DS3231.ZIP` | Драйвер часов реального времени DS3231/DS3232 |
| `HX711.zip` | Драйвер усилителя тензодатчика HX711, используется в `18_Tenzo` |
| `vl53l0x-arduino.zip` | Библиотека Pololu для дальномера VL53L0X |
| `SoftwareSerial.zip` | Программный UART, используется для радиомодуля HC-12 |
