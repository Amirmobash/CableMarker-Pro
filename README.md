# CableMarker Pro - ESP32-Based Cable Measuring and Marking System

![CableMarker Pro](docs/images/device-preview.jpg)

A professional-grade cable measuring and marking system using ESP32, laser distance sensor, and servo mechanism for precise cable marking applications.

## Features

- **Laser Distance Measurement**: High-precision laser ranging
- **Web Interface**: Responsive web-based control panel
- **Servo Marking Mechanism**: Automated physical marking
- **WiFi Access Point**: Standalone operation mode
- **Real-time Monitoring**: Live distance data and system status
- **Calibration System**: Offset calibration for accuracy

## Hardware Requirements

- ESP32 Development Board
- Laser Distance Sensor (UART interface)
- SG90 Servo Motor
- Buzzer & Status LEDs
- Trigger Input/Output pins
- Power Supply (5V/3.3V)

## Wiring

| Component | ESP32 Pin |
|-----------|-----------|
| Laser RX | GPIO 16 |
| Laser TX | GPIO 17 |
| Laser ENA | GPIO 5 |
| Servo | GPIO 18 |
| Buzzer | GPIO 27 |
| Status LED | GPIO 25 |
| Trigger Out | GPIO 23 |
| Trigger In | GPIO 19 |

## Installation

1. Clone this repository:
```bash
git clone https://github.com/yourusername/cable-marker-pro.git
