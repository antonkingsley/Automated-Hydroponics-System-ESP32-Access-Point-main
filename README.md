# Automated-Hydroponics-System-ESP32-Access-Point
Automated Hydroponics System ESP32 Access-Point (AP) for Web Server
# Automated Hydroponics System with ESP32 Web Server (AP Mode)

![Project Banner](https://via.placeholder.com/800x300.png/2c3e50/ffffff?text=Smart+Hydroponics+System)

An IoT-based automated hydroponics control system using ESP32 in Access Point (AP) mode. Provides real-time environmental monitoring and control through a responsive web interface.

## 🌟 Features

- **Multi-Sensor Monitoring**:
  - Air Temperature/Humidity (DHT22)
  - Water Level (Ultrasonic Sensor)
  - pH Level
  - Electroconductivity (EC)
  - Dissolved Oxygen

- **Actuator Control**:
  - Water Pump
  - Fertilizer Pump
  - Mist Generator
  - Solenoid Valve
  - Oxygen Pump

- **Web Interface**:
  - Real-time Dashboard with Charts
  - Manual Device Control
  - Threshold Configuration
  - Sensor Data Visualization
  - Mobile-Responsive Design

- **Automation**:
  - Climate-controlled irrigation
  - pH-balanced fertilization
  - Oxygen level maintenance
  - Water level management

## 🛠 Hardware Requirements

| Component               | Specification           |
|-------------------------|-------------------------|
| Microcontroller         | ESP32 Dev Board         |
| Temperature/Humidity    | DHT22 Sensor            |
| Water Level             | HC-SR04 Ultrasonic      |
| pH Sensor               | Analog pH Meter Pro     |
| EC Sensor               | Gravity Analog EC Sensor|
| Oxygen Sensor           | DFROBOT Dissolved O2    |
| Relays                  | 5V 4-Channel Relay      |
| LCD                     | 20x4 I2C Character LCD  |

**Pin Mapping**:
- DHT22 -> GPIO4
- Ultrasonic -> TRIG:5, ECHO:18
- pH Sensor -> GPIO35
- EC Sensor -> GPIO36
- Oxygen Sensor -> GPIO14
 
**Relays**:
- Water Pump -> 27
- Mist -> 32
- Solenoid -> 33
- Fertilizer -> 34
- Oxygen Pump -> 15

  
## ⚙️ Installation

1. **Arduino IDE Setup**
   - Install ESP32 Board Support
   - Install Required Libraries:
     - `WebServer`
     - `LiquidCrystal_I2C`
     - `DHT sensor library`

2. **Hardware Connection**
   - Connect sensors and actuators as per pin mapping
   - Power ESP32 via 5V/2A supply
   - Use waterproof enclosures for sensors

3. **Upload Code**
   - Clone repository
   - Open `Hydroponics_AP_WebServer.ino`
   - Upload to ESP32

## 📡 Usage

1. **Connect to ESP32 AP**
   - SSID: `HydroponicsAP`
   - Password: `password`
   - IP: `192.168.4.1` (Automatic)

2. **Web Interface Navigation**:
   - **Dashboard**: Real-time sensor graphs
   - **Manual Control**: Direct device operation
   - **Settings**: Configure thresholds
   - **Sensor Data**: Numerical readouts

3. **Default Thresholds**:
   - **Temperature**: 20-30°C
   - **Humidity**: 40-80%
   - **Water Level**: 10-50cm
   - **pH**: 6.0-7.5
   - **EC**: 1.0-2.5 mS/cm
   - **Oxygen**: 1.0-2.5%

## 🌐 API Endpoints

| Endpoint	          | Description             |
|-------------------------|-------------------------|
| /sensor-data            | HTML sensor display     |
| /sensor-data-json       | JSON sensor data        |
| /toggle-device          | Control devices (POST)  |
| /save-thresholds        | Update thresholds (POST)|

## 🚨 Troubleshooting

- **Common Issues**:
 
    - **Sensor inaccuracies**: Calibrate using reference solutions

    - **AP not appearing**: Check WiFi settings

    - **Relay not switching**: Verify power supply

    - **LCD not working**: Check I2C address

- **Calibration Guide**:
 
    - **pH**: Use pH4 and pH7 buffer solutions

    - **EC**: Calibrate with 1413µS/cm solution

    - **Oxygen**: Zero-point calibration in air

## 🤝 Contributing

     - Fork repository

     - Create feature branch (git checkout -b feature)

     - Commit changes (git commit -m 'Add feature')

     - Push to branch (git push origin feature)

     - Open Pull Request

## 📜 License

  - MIT License - See LICENSE for details

## 🙏 Acknowledgments

     - DFRobot for sensor libraries

     - Arduino community for code examples

     - PlatformIO for development tools


