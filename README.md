# ArduinoLab — Arduino Uno Sensor & Test Shield
<img src="https://github.com/pav2000/LabMaketUNO/blob/main/hard/Picture/main.png" width="640" /> <br>
<img src="https://github.com/pav2000/LabMaketUNO/blob/main/hard/Picture/view2.png" width="640" /> <br>

## Overview

**ArduinoLab** (rev. 1.2, 2025) is an Arduino Uno-compatible shield that combines
a display, a set of common sensors, buttons, a wireless module and several
peripheral ICs on a single board. It was designed as a learning / diagnostic
platform: a single board on which you can practice working with I2C sensors,
SPI displays, EEPROM, an RTC, a radio module and buttons — without wiring up
a breadboard.

The repository also contains a ready-made diagnostic firmware
(`ArduinoLab_Diagnostics.ino`) with a menu-driven test for every device on the
board, and a small standalone display library (`MiniTFT/`) written specifically
for this board's screen.

## Features

- On-screen menu navigated with three buttons (Up / OK / Down), showing live
  sensor readings that refresh once a second.
- Startup screen with firmware version, free RAM and an I2C bus scan.
- Individual, selectable test for every sensor and peripheral on the board.
- Compact custom ST7789 display driver (`MiniTFT` library) that does not
  depend on Adafruit_GFX / Adafruit_BusIO — useful on flash-constrained
  boards such as the Arduino Uno.
- Automatic detection of HMC5883L vs. QMC5883L compass chip (many "GY-273"
  modules actually carry a QMC5883L clone).

## Onboard components

| Ref. | Component | Interface | Purpose |
|---|---|---|---|
| TFT1 | ST7789 IPS display, 240×240, 1.3" | SPI | Menu and test results |
| U7 | HMC5883L / QMC5883L | I2C (0x1E or 0x0D) | 3-axis compass |
| U12 | VL53L0X | I2C (0x29) | Time-of-flight distance sensor |
| U14 | DS3232 | I2C (0x68) | Real-time clock (with CR2032 backup battery, BT1) |
| U13 | AT24C32 | I2C (0x50) | 32 Kbit EEPROM |
| U9 (labelled DHT11 on board) | DHT11 | 1-wire (digital) | Temperature & humidity |
| — | LM35 | Analog (A2) | Temperature sensor |
| — | Photoresistor (LDR) | Analog (A1) | Light level sensor |
| — | Potentiometer | Analog (A0) | Variable-resistor / general analog input |
| U16 | HC-12 | UART (SoftwareSerial) | 433 MHz wireless transceiver |
| SG1 | Piezo buzzer | Digital (D5), via transistor | Sound output |
| — | Addressable RGB LED | Digital | Status indication |
| SW1 / SW2 / SW5 | Up / OK / Down buttons | Analog ladder (A3) | Menu navigation |
| SW3 / SW4 | Push buttons | Digital (D2 / D3) | General-purpose input test |
| U1 | AMS1117-3.3 | — | 3.3 V regulator for I2C sensors / display logic |

Additional breakouts: full analog pin header (A0–A5), power header
(IOREF / RESET / 3V3 / 5V / GND / VIN), and spare digital pin headers
(D7 / D8, with +5V/GND) for expansion modules.

## Related files in this repository

- `ArduinoLab_Diagnostics.ino` — menu-driven diagnostic firmware for the whole board.
- `MiniTFT/` — standalone Arduino library for the on-board ST7789 display.

---

# ArduinoLab — плата-шилд для Arduino Uno с датчиками и тестами

<img src="https://github.com/pav2000/LabMaketUNO/blob/main/hard/Picture/main.png" width="640" /> <br>
<img src="https://github.com/pav2000/LabMaketUNO/blob/main/hard/Picture/view2.png" width="640" /> <br>

## Обзор

**ArduinoLab** (ревизия 1.2, 2025) — это плата-шилд, совместимая с Arduino Uno,
которая объединяет на одной плате экран, набор распространённых датчиков,
кнопки, беспроводной модуль и несколько периферийных микросхем. Плата
задумана как учебная/диагностическая платформа: одна плата, на которой можно
практиковаться в работе с I2C-датчиками, SPI-экраном, EEPROM, часами
реального времени, радиомодулем и кнопками — без макетной платы и проводов.

В этом же репозитории лежит готовая диагностическая прошивка
(`ArduinoLab_Diagnostics.ino`) с меню и тестом для каждого устройства на
плате, а также небольшая отдельная библиотека для экрана (`MiniTFT/`),
написанная специально под экран этой платы.

## Возможности

- Меню на экране с навигацией тремя кнопками (Up / OK / Down), «живые»
  показания датчиков обновляются раз в секунду.
- Стартовый экран с версией прошивки, объёмом свободной RAM и результатами
  сканирования шины I2C.
- Отдельный, выбираемый из меню тест для каждого датчика и узла платы.
- Компактный собственный драйвер экрана ST7789 (библиотека `MiniTFT`) без
  зависимости от Adafruit_GFX / Adafruit_BusIO — полезно на платах с
  ограниченной флеш-памятью, таких как Arduino Uno.
- Автоматическое определение реального чипа компаса — HMC5883L или
  QMC5883L (многие модули "GY-273" на деле содержат клон QMC5883L).

## Установленные на плате устройства

| Обозн. | Компонент | Интерфейс | Назначение |
|---|---|---|---|
| TFT1 | IPS-экран ST7789, 240×240, 1.3" | SPI | Меню и результаты тестов |
| U7 | HMC5883L / QMC5883L | I2C (0x1E или 0x0D) | 3-осевой компас |
| U12 | VL53L0X | I2C (0x29) | Лазерный дальномер (time-of-flight) |
| U14 | DS3232 | I2C (0x68) | Часы реального времени (с батарейкой CR2032, BT1) |
| U13 | AT24C32 | I2C (0x50) | EEPROM на 32 Кбит |
| U9 (подписан DHT11 на плате) | DHT11 | 1-wire (цифровой) | Температура и влажность |
| — | LM35 | Аналоговый (A2) | Датчик температуры |
| — | Фоторезистор (LDR) | Аналоговый (A1) | Датчик освещённости |
| — | Потенциометр | Аналоговый (A0) | Переменный резистор / общий аналоговый вход |
| U16 | HC-12 | UART (SoftwareSerial) | Беспроводной трансивер 433 МГц |
| SG1 | Пьезозуммер | Цифровой (D5), через транзистор | Звуковой выход |
| — | Адресный RGB-светодиод | Цифровой | Индикация состояния |
| SW1 / SW2 / SW5 | Кнопки Up / OK / Down | Аналоговая лесенка (A3) | Навигация по меню |
| SW3 / SW4 | Кнопки | Цифровые (D2 / D3) | Тест обычного цифрового входа |
| U1 | AMS1117-3.3 | — | Стабилизатор 3.3В для I2C-датчиков/логики экрана |

Дополнительно на плате разведены: полный набор аналоговых пинов (A0–A5),
разъём питания (IOREF / RESET / 3V3 / 5V / GND / VIN) и запасные цифровые
пины (D7 / D8, с выводами +5V/GND) для подключения дополнительных модулей.

## Связанные файлы в этом репозитории

- `ArduinoLab_Diagnostics.ino` — диагностическая прошивка с меню для всей платы.
- `MiniTFT/` — отдельная Arduino-библиотека для установленного на плате экрана ST7789.
