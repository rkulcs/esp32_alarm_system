# ESP32 Alarm System

This is a simple alarm system that I created to learn about embedded systems programming using an ESP32 microcontroller and various peripherals. The system uses an ultrasonic ranging module to detect potential intruders. 

When the system is initialized, the sensor is used to calculate the distance to object that is closest to it. Different samples are taken to set a range of "safe distances". Afterwards, the ranging module used to periodically computee the distance between the system and the nearest object. If multiple distances which fall outside of the range of safe distances are recorded within a short period (e.g., when someone gets too close to the alarm system), then the alarm is triggered.

### Requirements

**ESP32 Alarm System**
- [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html#installation)

- Hardware components:
  - x1 ESP32 WROVER
  - x1 Red LED
  - x1 LCD1602
  - x1 HC-SR04 ultrasonic ranging module
  - x1 5V active buzzer
  - x1 S8050 NPN transistor
  - Resistors:
    - x1 220 Ω
    - x1 1 kΩ
    - x2 10 kΩ

**Android Application**
- [Firebase](https://firebase.google.com)
- [Android Studio](https://developer.android.com/studio)

### Circuit

<img src="https://github.com/rkulcs/esp32_alarm_system/assets/50153954/e0d8ac77-84f5-4762-8ceb-49f86a10c336" width="60%" />
