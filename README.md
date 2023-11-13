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

<img src="https://github.com/rkulcs/esp32_alarm_system/assets/50153954/8f3fbc00-0a55-4838-a028-ee3fea954477" width="60%" />

### Installation and Configuration

**Firebase Application and User Account**

1. Go to the [Firebase Console](https://console.firebase.google.com), and create a new project.
2. Go to authentication, and create a new application user account. This account does not require the use of a real email address or phone number.

**Spring Boot Server Application**

1. Find the Firebase project's web API key in the project settings.
2. Edit the *server/src/main/resources/application.properties* file of the Spring Boot project to update the server IP and port number if needed, and to add the Firebase web API key.
3. Under the *Service accounts* tab of the Firebase project settings, select *Firebase Admin SDK*, then click on *Generate new private key*. This will download a JSON file.
4. Rename the JSON file to *firebase-service-account.json*, and move it to *server/src/main/resources/application.properties*.
5. Build and run the server.
   ```console
   $ cd server
   $ ./mvnw spring-boot:run
   ```

**ESP32 Alarm System**

1. Add the ESP-IDF tools to the path.
   ```console
   $ get_idf
   ```
2. Go to the *esp32* directory, and open the configuration menu.
   ```console
   $ cd esp32
   $ idf.py menuconfig
   ```
   - Choose the "WiFi Configuration" option, and enter the credentials of the WiFi network to use.
   - In the "Remote Server Configuration" submenu, set the IP address and port number of the Spring Boot server.
   - In the "Firebase Authentication" submenu, set the email address and password of the Firebase application user account.
3. Build the application and flash the ESP32 WROVER.
   ```console
   $ idf.py build flash
   ```

**Android Application**

1. In the project overview of the Firebase Console, click on the *Add app* button.
2. Add a new Android application with the package name *alarm.system.app*, and follow the setup instructions.
3. Open and build the project using Android Studio.
