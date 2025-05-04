# Wifi-Controlled-Car
# Wifi-Controlled-Car

This project allows you to control a car using Wi-Fi through an ESP8266 microcontroller. It connects to a Wi-Fi network and listens for commands to move the car in different directions, adjust its speed, and perform other actions like turning on a buzzer.

## Features
- **Wi-Fi Control**: Connects to a Wi-Fi network and listens for commands over a server.
- **Motor Control**: Controls two DC motors for forward, backward, left, and right movement.
- **Speed Control**: Adjusts motor speed using Pulse Width Modulation (PWM).
- **Buzzer**: Optional sound alert using a buzzer.
- **Responsive Commands**: Receives commands over a web interface or directly via a connected device.

## Components
- **ESP8266 (NodeMCU)**: Used to handle Wi-Fi connection and motor control.
- **DC Motors**: Controlled via the ESP8266 pins using H-Bridge.
- **Buzzer**: Alerts or plays sounds (optional).
- **Power Supply**: Powers the motors and the ESP8266.

## Pin Configuration
- **Motor Driver Pins**:
  - IN1: D1
  - IN2: D2
  - IN3: D3
  - IN4: D4
  - ENA: D0 (Left motor speed control)
  - ENB: D5 (Right motor speed control)
  - BUZZER: D6 (Buzzer control)

## Setup

### 1. Install Arduino IDE
Make sure you have the Arduino IDE installed and configured for the ESP8266 platform.

### 2. Install Libraries
You will need to install the ESP8266WiFi library:
- Go to **Sketch** > **Include Library** > **Manage Libraries...**
- Search for **ESP8266WiFi** and install it.

### 3. Code Configuration
- Set your Wi-Fi credentials in the code:
  ```cpp
  const char* ssid = "YourWiFiName";
  const char* password = "YourWiFiPassword";
