#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// Constants
#define DHTPIN 4 // Humidity and temperature pin 4
#define DHTTYPE DHT22
#define TRIG_PIN 5
#define ECHO_PIN 18
#define RELAY_PUMP 27
#define RELAY_MIST 32
#define RELAY_SOLENOID 33
#define RELAY_FERTILIZER 34
#define PH_SENSOR_PIN 35
#define EC_SENSOR_PIN 36
#define OXYGEN_SENSOR_PIN 14
#define RELAY_OXYGEN_PUMP 15
const float calibrationFactor = 1.0; // Calibration factor for your EC sensor
const float tempCoefficient = 0.019; // Temperature coefficient

const float vRef = 3.3;                     // Reference voltage for ESP32
const int adcResolution = 4095;             // ESP32 ADC resolution (12-bit)
const float OxygentempCoefficient = 0.03;        // Temperature coefficient for Oxygen compensation

#define LCD_ADDRESS 0x27

// Create instances
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(LCD_ADDRESS, 20, 4);
WebServer server(80);

// Wi-Fi credentials (Access Point Mode)
String ssid = "HydroponicsAP"; // AP SSID
String password = "password"; // AP Password


// Static IP Configuration
IPAddress apIP(192, 168, 4, 1);     // Desired IP address for the AP
IPAddress apGateway(192, 168, 4, 1); // Gateway (usually same as IP)
IPAddress apSubnet(255, 255, 255, 0); // Subnet mask

void setupWiFiAP() {
  Serial.println("Configuring access point...");
  WiFi.softAPConfig(apIP, apGateway, apSubnet); // Configure static IP
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
}

// Thresholds
float tempUpperLimit = 30.0;
float tempLowerLimit = 20.0;
float humidityUpperLimit = 80.0;
float humidityLowerLimit = 40.0;
float waterUpperLimit = 10.0; // cm
float waterLowerLimit = 50.0; // cm
float pHUpperLimit = 7.5; // pH upper limit
float pHLowerLimit = 6.0; // pH lower limit
float ecUpperLimit = 2.5; // Electroconductivity upper limit (mS/cm)
float ecLowerLimit = 1.0; // Electroconductivity lower limit (mS/cm)
float oxygenUpperLimit = 2.5; // oxygen saturation levels  upper limit (%)
float oxygenLowerLimit = 1.0; // oxygen saturation levels  lower limit (%)

// Device states
bool pumpState = false;
bool mistState = false;
bool solenoidState = false;
bool fertilizerState = false;
bool oxygenState = false;



// Function prototypes
void handleRoot();
void handleThresholdSettings();
void handleManualControl();
void handleSensorData();
void handleSensorDataJSON();
void handleDashboard();
void handleSaveThresholds();
void handleToggleDevice();
void handleNotFound();
void setupWiFiAP(); // Changed to AP setup
void setupWebServer();
void updateLCD();
float measureWaterLevel();
float readPHSensor();
float readECSensor();
float readOxygenSensor();

void handleFertilization();
void handleWaterSolenoid();
void handleWaterPump();
void handleMistPump();
void handleOxygenPump();





void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RELAY_PUMP, OUTPUT);
  pinMode(RELAY_MIST, OUTPUT);
  pinMode(RELAY_SOLENOID, OUTPUT);
  pinMode(RELAY_FERTILIZER, OUTPUT);
  pinMode(RELAY_OXYGEN_PUMP, OUTPUT);

  pinMode(PH_SENSOR_PIN, INPUT);
  pinMode(EC_SENSOR_PIN, INPUT);
  pinMode(OXYGEN_SENSOR_PIN, INPUT);  
  
  lcd.init();
  lcd.backlight();
  setupWiFiAP(); // Changed to AP setup
  setupWebServer();
}

void loop() {
  server.handleClient();
  updateLCD();
  handleFertilization();
  handleWaterSolenoid();
  handleWaterPump();
  handleMistPump();
  handleOxygenPump();
  delay(1000);
}

// Web server setup
void setupWebServer() {
  server.on("/", handleRoot);
  server.on("/threshold-settings", handleThresholdSettings);
  server.on("/manual-control", handleManualControl);
  server.on("/sensor-data", handleSensorData);
  server.on("/sensor-data-json", handleSensorDataJSON);
  server.on("/dashboard", handleDashboard);
  server.on("/save-thresholds", HTTP_POST, handleSaveThresholds);
  server.on("/toggle-device", HTTP_POST, handleToggleDevice);
  server.onNotFound(handleNotFound);
  server.begin();
}

// Handle root page
void handleRoot() {
  String html = R"rawliteral(
   <!DOCTYPE html>
  <html lang="en">
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Hydroponics Dashboard</title>
 <link href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/css/all.min.css" rel="stylesheet">
    <style>
      body { font-family: Arial, sans-serif; background-color: #f4f4f4; margin: 0; padding: 0; }
      h1 { text-align: center; color: #4CAF50; margin: 20px; }
      .container { max-width: 100%; margin: 20px auto; background: #fff; border-radius: 10px; padding: 20px; box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1); }
      .nav { display: flex; justify-content: space-around; margin-bottom: 20px; }
      .nav a { text-decoration: none; color: #4CAF50; font-weight: bold; }
      .button {
  display: inline-block;
  padding: 12px 24px;
  font-size: 18px;
  font-weight: bold;
  text-align: center;
  text-decoration: none;
  color: #4CAF50;
  border: 2px solid #4CAF50;
  border-radius: 25px;
  background-color: transparent;
  cursor: pointer;
  transition: all 0.4s ease;
  box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
}

.button:hover {
  background-color: #4CAF50;
  color: #ffffff;
  box-shadow: 0 6px 12px rgba(0, 0, 0, 0.2);
  transform: translateY(-2px);
}

.button:active {
  transform: translateY(1px);
  box-shadow: 0 3px 6px rgba(0, 0, 0, 0.15);
}

    </style>
  </head>
  <body>
    <h1>Hydroponics Dashboard</h1>
    <div class="container">
      <div class="nav">
        <a href="/" class="button">Home</a>
     <a href="/dashboard" class="button">Dashboard</a>
        <a href="/manual-control" class="button">Manual</a>
        <a href="/threshold-settings" class="button">Settings</a>
         <a href="/sensor-data" class="button">Sensor Data</a>
        
      </div>
    
    <div style="display: flex; justify-content: center; align-items: center; height: 100vh;">
    <svg width="100%" height="auto" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1200 600">
        <!-- Background and Ground -->
        <rect width="100%" height="100%" fill="#c7f0f9" />
        <rect y="450" width="100%" height="150" fill="#5DB996" />

        <!-- Greenhouse Structure -->
        <svg viewBox="0 0 400 400" xmlns="http://www.w3.org/2000/svg">
    <!-- Greenhouse Structure -->
    <path d="M50,200 Q250,-50 450,200 L450,350 H50 Z" fill="lightblue" stroke="darkblue" stroke-width="2"/>
    <line x1="250" y1="75" x2="250" y2="350" stroke="darkblue" stroke-width="2"/>
    
    <!-- Tomato Plants -->
    <g transform="translate(120,220)">
        <rect x="-10" y="60" width="20" height="40" fill="brown"/>
        <circle cx="0" cy="50" r="25" fill="green"/>
        <circle cx="-10" cy="40" r="8" fill="yellow"/>
        <circle cx="10" cy="45" r="8" fill="red"/>
        <circle cx="0" cy="30" r="8" fill="#4df04d"/>
    </g>
    
    <g transform="translate(250,220)">
        <rect x="-10" y="60" width="20" height="40" fill="brown"/>
        <circle cx="0" cy="50" r="25" fill="green"/>
        <circle cx="-10" cy="40" r="8" fill="red"/>
        <circle cx="10" cy="45" r="8" fill="#4ffbdf"/>
        <circle cx="0" cy="30" r="8" fill="#fefedf"/>
    </g>
    
    <g transform="translate(380,220)">
        <rect x="-10" y="60" width="20" height="40" fill="brown"/>
        <circle cx="0" cy="50" r="25" fill="green"/>
        <circle cx="-10" cy="40" r="8" fill="red"/>
        <circle cx="10" cy="45" r="8" fill="#f9f871"/>
        <circle cx="0" cy="30" r="8" fill="#4dff4d"/>
    </g>
</svg>

       

        <!-- Control Panel -->
        <rect x="50" y="450" width="250" height="150" rx="20" fill="#2c3e50" stroke="#1976D2" stroke-width="5" />
        <circle cx="100" cy="500" r="20" fill="#42A5F5" />
        <circle cx="150" cy="500" r="20" fill="#FF7043" />
        <circle cx="200" cy="500" r="20" fill="#FFEB3B" />
        <circle cx="250" cy="500" r="20" fill="#8BC34A" />
        
        <!-- Text -->
        <text x="50" y="100" font-family="Arial" font-size="40" fill="#2c3e50" font-weight="bold">SMART GREENHOUSES:</text>
        <text x="30" y="150" font-family="Arial" font-size="25" fill="#4CAF50">IoT Technologies for Controlled Environments</text>
    </svg>
</div>

    </div>
  </body>
  </html>
  )rawliteral";
  server.send(200, "text/html", html);
}

// Handle dashboard page with graphs
void handleDashboard() {
  String html = R"rawliteral(
  <!DOCTYPE html>
  <html lang="en">
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Dashboard</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
      body { font-family: Arial, sans-serif; background-color: #f4f4f4; margin: 0; padding: 0; }
      h1 { text-align: center; color: #4CAF50; margin: 20px; }
      .container { max-width: 100%; margin: 20px auto; background: #fff; border-radius: 10px; padding: 20px; box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1); }
      .nav { display: flex; justify-content: space-around; margin-bottom: 20px; }
      .nav a { text-decoration: none; color: #4CAF50; font-weight: bold; }
      .chart-container { margin: 20px 0; }
      .button {
  display: inline-block;
  padding: 12px 24px;
  font-size: 18px;
  font-weight: bold;
  text-align: center;
  text-decoration: none;
  color: #4CAF50;
  border: 2px solid #4CAF50;
  border-radius: 25px;
  background-color: transparent;
  cursor: pointer;
  transition: all 0.4s ease;
  box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
}

.button:hover {
  background-color: #4CAF50;
  color: #ffffff;
  box-shadow: 0 6px 12px rgba(0, 0, 0, 0.2);
  transform: translateY(-2px);
}

.button:active {
  transform: translateY(1px);
  box-shadow: 0 3px 6px rgba(0, 0, 0, 0.15);
}

    </style>
  </head>
  <body>
    <h1>Dashboard</h1>
   <div class="container">
      <div class="nav">
        <a href="/" class="button">Home</a>
     <a href="/dashboard" class="button">Dashboard</a>
        <a href="/manual-control" class="button">Manual</a>
         <a href="/threshold-settings" class="button">Settings</a>
         <a href="/sensor-data" class="button">Sensor Data</a>        
      </div>
      <div class="chart-container">
        <canvas id="sensorChart"></canvas>
      </div>
    </div>
    <script>
      const ctx = document.getElementById('sensorChart').getContext('2d');
      const chart = new Chart(ctx, {
        type: 'line',
        data: {
          labels: [],
          datasets: [
            { label: 'Temperature (°C)', borderColor: '#FF6384', data: [] },
            { label: 'Humidity (%)', borderColor: '#36A2EB', data: [] },
            { label: 'Water Level (cm)', borderColor: '#4BC0C0', data: [] },
            { label: 'pH Value', borderColor: '#FFCE56', data: [] },
            { label: 'EC Value (mS/cm)', borderColor: '#9966FF', data: [] }
            { label: 'oxygen Value (%)', borderColor: '#6699FF', data: [] }

          ]
        },
        options: { responsive: true, scales: { y: { beginAtZero: true } } }
      });

      function fetchSensorData() {
        fetch('/sensor-data')
          .then(response => response.text())
          .then(html => {
            const parser = new DOMParser();
            const doc = parser.parseFromString(html, 'text/html');
            const temperature = parseFloat(doc.getElementById('temperature').textContent);
            const humidity = parseFloat(doc.getElementById('humidity').textContent);
            const waterLevel = parseFloat(doc.getElementById('waterLevel').textContent);
            const pHValue = parseFloat(doc.getElementById('pHValue').textContent);
            const ecValue = parseFloat(doc.getElementById('ecValue').textContent);
            const oxygenValue = parseFloat(doc.getElementById('oxygenValue').textContent);


            const now = new Date();
            const time = now.toLocaleTimeString();

            chart.data.labels.push(time);
            chart.data.datasets[0].data.push(temperature);
            chart.data.datasets[1].data.push(humidity);
            chart.data.datasets[2].data.push(waterLevel);
            chart.data.datasets[3].data.push(pHValue);
            chart.data.datasets[4].data.push(ecValue);
            chart.data.datasets[5].data.push(oxygenValue);

            if (chart.data.labels.length > 15) {
              chart.data.labels.shift();
              chart.data.datasets.forEach(dataset => dataset.data.shift());
            }

            chart.update();
          });
      }

      setInterval(fetchSensorData, 5000); // Refresh every 5 seconds
    </script>
 
    
</div>
  </body>
  </html>
  )rawliteral";
  server.send(200, "text/html", html);
}


// Handle threshold settings page
void handleThresholdSettings() {
  String html = R"rawliteral(
  <!DOCTYPE html>
  <html lang="en">
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Threshold Settings</title>
     <link href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/css/all.min.css" rel="stylesheet">
    <style>
      body { font-family: Arial, sans-serif; background-color: #f4f4f4; margin: 0; padding: 0; }
      h1 { text-align: center; color: #4CAF50; margin: 20px; }
      .container { max-width: 100%; margin: 20px auto; background: #fff; border-radius: 10px; padding: 20px; box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1); }
      .form-group { margin-bottom: 15px; }
      .form-group label { display: block; margin-bottom: 5px; font-weight: bold; }
      .form-group input { width: 100%; padding: 10px; border: 1px solid #ddd; border-radius: 5px; }
      .form-group button { padding: 10px 20px; background-color: #4CAF50; color: white; border: none; border-radius: 5px; cursor: pointer; }
      .nav { display: flex; justify-content: space-around; margin-bottom: 20px; }
      .nav a { text-decoration: none; color: #4CAF50; font-weight: bold; }

  .button {
  display: inline-block;
  padding: 12px 24px;
  font-size: 18px;
  font-weight: bold;
  text-align: center;
  text-decoration: none;
  color: #4CAF50;
  border: 2px solid #4CAF50;
  border-radius: 25px;
  background-color: transparent;
  cursor: pointer;
  transition: all 0.4s ease;
  box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
}

.button:hover {
  background-color: #4CAF50;
  color: #ffffff;
  box-shadow: 0 6px 12px rgba(0, 0, 0, 0.2);
  transform: translateY(-2px);
}

.button:active {
  transform: translateY(1px);
  box-shadow: 0 3px 6px rgba(0, 0, 0, 0.15);
}

      </style>  
  </head>
  <body>
    <h1>Settings</h1>
    <div class="container">
      <div class="nav">
        <a href="/" class="button">Home</a>
     <a href="/dashboard" class="button">Dashboard</a>
        <a href="/manual-control" class="button">Manual</a>
        <a href="/threshold-settings" class="button">Settings</a>
         <a href="/sensor-data" class="button">Sensor Data</a>
        
      </div>   
    <form action="/save-thresholds" method="post">
      <label>Temp Upper:</label><input type="number" name="tempUpper" value=")rawliteral" + String(tempUpperLimit) + R"rawliteral("><br>
      <label>Temp Lower:</label><input type="number" name="tempLower" value=")rawliteral" + String(tempLowerLimit) + R"rawliteral("><br>
      <label>Humidity Upper:</label><input type="number" name="humidityUpper" value=")rawliteral" + String(humidityUpperLimit) + R"rawliteral("><br>
      <label>Humidity Lower:</label><input type="number" name="humidityLower" value=")rawliteral" + String(humidityLowerLimit) + R"rawliteral("><br>
      <label>Water Upper:</label><input type="number" name="waterUpper" value=")rawliteral" + String(waterUpperLimit) + R"rawliteral("><br>
      <label>Water Lower:</label><input type="number" name="waterLower" value=")rawliteral" + String(waterLowerLimit) + R"rawliteral("><br>
      <label>pH Upper:</label><input type="number" name="pHUpper" value=")rawliteral" + String(pHUpperLimit) + R"rawliteral("><br>
      <label>pH Lower:</label><input type="number" name="pHLower" value=")rawliteral" + String(pHLowerLimit) + R"rawliteral("><br>
      <label>EC Upper:</label><input type="number" name="ecUpper" value=")rawliteral" + String(ecUpperLimit) + R"rawliteral("><br>
      <label>EC Lower:</label><input type="number" name="ecLower" value=")rawliteral" + String(ecLowerLimit) + R"rawliteral("><br>
      <label>Oxygen Upper:</label><input type="number" name="oxygenUpper" value=")rawliteral" + String(oxygenUpperLimit) + R"rawliteral("><br>
      <label>Oxygen Lower:</label><input type="number" name="oxygenLower" value=")rawliteral" + String(oxygenLowerLimit) + R"rawliteral("><br>
     
      <input type="submit" value="Save">
    </form>
  </body>
  </html>
  )rawliteral";
  server.send(200, "text/html", html);
}


// -------------------
// Handle manual control page
void handleManualControl() {
  String html = R"rawliteral(
  <!DOCTYPE html>
  <html lang="en">
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Manual Control</title>
    <link href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/css/all.min.css" rel="stylesheet">
    <style>
      body { font-family: Arial, sans-serif; background-color: #f4f4f4; margin: 0; padding: 0; }
      h1 { text-align: center; color: #4CAF50; margin: 20px; }
      .container { max-width: 100%; margin: 20px auto; background: #fff; border-radius: 10px; padding: 20px; box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1); }
      .device-control { display: flex; justify-content: space-between; align-items: center; margin: 20px 0; padding: 10px 20px; background: #f9f9f9; border: 1px solid #ddd; border-radius: 8px; box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1); }
      .device-name { font-size: 18px; font-weight: bold; color: #333; display: flex; align-items: center; }
      .device-name i { margin-right: 10px; color: #4CAF50; }
      .button { padding: 10px 20px; font-size: 16px; border: none; border-radius: 5px; cursor: pointer; color: white; }
      .on { background-color: #4CAF50; }
      .off { background-color: #f44336; }
      .nav { display: flex; justify-content: space-around; margin-bottom: 20px; }
      .nav a { text-decoration: none; color: #4CAF50; font-weight: bold; }

  .button {
  display: inline-block;
  padding: 12px 24px;
  font-size: 18px;
  font-weight: bold;
  text-align: center;
  text-decoration: none;
  color: #4CAF50;
  border: 2px solid #4CAF50;
  border-radius: 25px;
  background-color: transparent;
  cursor: pointer;
  transition: all 0.4s ease;
  box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
}

.button:hover {
  background-color: #4CAF50;
  color: #ffffff;
  box-shadow: 0 6px 12px rgba(0, 0, 0, 0.2);
  transform: translateY(-2px);
}

.button:active {
  transform: translateY(1px);
  box-shadow: 0 3px 6px rgba(0, 0, 0, 0.15);
}

.manualbutton {
  display: inline-block;
  padding: 12px 24px;
  font-size: 18px;
  font-weight: bold;
  text-align: center;
  text-decoration: none;
  color: #ff0000;
  border: 2px solid #ff0000;
  border-radius: 25px;
  background-color: transparent;
  cursor: pointer;
  transition: all 0.4s ease;
  box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
}

.manualbutton:hover {
  background-color: #ff0000;
  color: #ffffff;
  box-shadow: 0 6px 12px rgba(0, 0, 0, 0.2);
  transform: translateY(-2px);
}

.manualbutton:active {
  transform: translateY(1px);
  box-shadow: 0 3px 6px rgba(0, 0, 0, 0.15);
}

    </style>
  </head>
  <body>
    <h1>Manual Control</h1>
    <div class="container">
     <div class="nav">
        <a href="/" class="button">Home</a>    
         <a href="/dashboard" class="button">Dashboard</a>  
     <a href="/manual-control" class="button">Manual</a>
         <a href="/sensor-data" class="button">Sensor Data</a>
    
      </div>

      <div class="device-control">
        <div class="device-name">
          <i class="fas fa-water"></i> Water Pump
        </div>
        <button class="manualbutton off" id="waterPump" onclick="toggleDevice('waterPump')">OFF</button>
      </div>
      <div class="device-control">
        <div class="device-name">
          <i class="fas fa-seedling"></i> Fertilizer Pump
        </div>
        <button class="manualbutton off" id="fertilizerPump" onclick="toggleDevice('fertilizerPump')">OFF</button>
      </div>
      <div class="device-control">
        <div class="device-name">
          <i class="fas fa-cloud"></i> Mist Generator
        </div>
        <button class="manualbutton off" id="mistGenerator" onclick="toggleDevice('mistGenerator')">OFF</button>
      </div>
      <div class="device-control">
        <div class="device-name">
          <i class="fas fa-tint"></i> Solenoid Valve
        </div>
        <button class="manualbutton off" id="solenoidValve" onclick="toggleDevice('solenoidValve')">OFF</button>
      </div>
      <div class="device-control">
        <div class="device-name">
          <i class="fas fa-wind"></i> Oxygen Pump
        </div>
        <button class="manualbutton off" id="oxygenValve" onclick="toggleDevice('oxygenValve')">OFF</button>
      </div>
    </div>
    <script>
      function toggleDevice(deviceId) {
        fetch(`/toggle-device?device=${deviceId}`, { method: 'POST' })
          .then(response => response.text())
          .then(data => {
            const button = document.getElementById(deviceId);
            if (data === "ON") {
              button.textContent = "ON";
              button.classList.remove("off");
              button.classList.add("on");
            } else {
              button.textContent = "OFF";
              button.classList.remove("on");
              button.classList.add("off");
            }
          });
      }
    </script>
  </body>
  </html>
  )rawliteral";
  server.send(200, "text/html", html);
}

// Handle sensor data page

void handleSensorData() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  float waterLevel = measureWaterLevel();
  float pHValue = readPHSensor();
  float ecValue = readECSensor();
  float oxygenValue = readOxygenSensor();

  String html = R"rawliteral(
  <!DOCTYPE html>
  <html lang="en">
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Sensor Data</title>
    <link href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/css/all.min.css" rel="stylesheet">
    <style>
      body {
        font-family: 'Arial', sans-serif;
        background-color: #f4f4f9;
        margin: 0;
        padding: 0;
        display: flex;
        justify-content: center;
        align-items: center;
        min-height: 100vh;
      }
      .container {
        background: #ffffff;
        border-radius: 12px;
        box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);
        padding: 30px;
        max-width: 100%;
        width: 100%;
        margin: 20px;
      }
      h1 {
        text-align: center;
        color: #4CAF50;
        margin-bottom: 20px;
        font-size: 28px;
        font-weight: bold;
      }
      .nav {
        display: flex;
        justify-content: space-around;
        margin-bottom: 30px;
      }
      .nav a {
        text-decoration: none;
        color: #4CAF50;
        font-weight: bold;
        font-size: 16px;
        transition: color 0.3s ease;
      }
      .nav a:hover {
        color: #45a049;
      }
      .sensor-card {
        background: #f9f9f9;
        border-radius: 10px;
        padding: 20px;
        margin-bottom: 20px;
        box-shadow: 0 2px 8px rgba(0, 0, 0, 0.05);
        transition: transform 0.3s ease, box-shadow 0.3s ease;
      }
      .sensor-card:hover {
        transform: translateY(-5px);
        box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);
      }
      .sensor-card label {
        font-size: 18px;
        font-weight: bold;
        color: #333;
        display: block;
        margin-bottom: 10px;
      }
      .sensor-card span {
        font-size: 24px;
        color: #4CAF50;
        font-weight: bold;
      }
      .sensor-card i {
        margin-right: 10px;
        color: #4CAF50;
      }

      .button {<style>
  display: inline-block;
  padding: 12px 24px;
  font-size: 18px;
  font-weight: bold;
  text-align: center;
  text-decoration: none;
  color: #4CAF50;
  border: 2px solid #4CAF50;
  border-radius: 25px;
  background-color: transparent;
  cursor: pointer;
  transition: all 0.4s ease;
  box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
}

.button:hover {
  background-color: #4CAF50;
  color: #ffffff;
  box-shadow: 0 6px 12px rgba(0, 0, 0, 0.2);
  transform: translateY(-2px);
}

.button:active {
  transform: translateY(1px);
  box-shadow: 0 3px 6px rgba(0, 0, 0, 0.15);
}

    </style>
  </head>
  <body>
    <div class="container">
      <h1>Sensor Data</h1>
      <div class="nav">
        <a href="/" class="button">Home</a>
     <a href="/dashboard" class="button">Dashboard</a>
        <a href="/manual-control" class="button">Manual</a>
         <a href="/sensor-data" class="button">Sensor Data</a>
      
      </div>
      <div class="sensor-card">
        <label><i class="fas fa-thermometer-half"></i>Temperature</label>
        <span id="temperature">)rawliteral" + String(temperature) + R"rawliteral( °C</span>
      </div>
      <div class="sensor-card">
        <label><i class="fas fa-tint"></i>Humidity</label>
        <span id="humidity">)rawliteral" + String(humidity) + R"rawliteral( %</span>
      </div>
      <div class="sensor-card">
        <label><i class="fas fa-ruler-vertical"></i>Water Level</label>
        <span id="waterLevel">)rawliteral" + String(waterLevel) + R"rawliteral( cm</span>
      </div>
      <div class="sensor-card">
        <label><i class="fas fa-flask"></i>pH Value</label>
        <span id="pHValue">)rawliteral" + String(pHValue) + R"rawliteral(</span>
      </div>
      <div class="sensor-card">
        <label><i class="fas fa-bolt"></i>EC Value</label>
        <span id="ecValue">)rawliteral" + String(ecValue) + R"rawliteral( mS/cm</span>
      </div>
       <div class="sensor-card">
        <label><i class="fas fa-bolt"></i>Oxygen Value</label>
        <span id="oxygenValue">)rawliteral" + String(oxygenValue) + R"rawliteral( %</span>
      </div>
    </div>
    <script>
      function fetchSensorData() {
        fetch('/sensor-data-json')
          .then(response => response.json())
          .then(data => {
            document.getElementById('temperature').textContent = data.temperature.toFixed(2) + ' °C';
            document.getElementById('humidity').textContent = data.humidity.toFixed(2) + ' %';
            document.getElementById('waterLevel').textContent = data.waterLevel.toFixed(2) + ' cm';
            document.getElementById('pHValue').textContent = data.pHValue.toFixed(2);
            document.getElementById('ecValue').textContent = data.ecValue.toFixed(2) + ' mS/cm';
            document.getElementById('oxygenValue').textContent = data.oxygenValue.toFixed(2) + ' %';

          });
      }

      // Refresh sensor data every 5 seconds
      setInterval(fetchSensorData, 5000);
    </script>
  </body>
  </html>
  )rawliteral";
  server.send(200, "text/html", html);
}

// Handle handleSensorDataJSON

void handleSensorDataJSON() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  float waterLevel = measureWaterLevel();
  float pHValue = readPHSensor();
  float ecValue = readECSensor();
  float oxygenValue = readOxygenSensor();

  String json = "{";
  json += "\"temperature\":" + String(temperature) + ",";
  json += "\"humidity\":" + String(humidity) + ",";
  json += "\"waterLevel\":" + String(waterLevel) + ",";
  json += "\"pHValue\":" + String(pHValue) + ",";
  json += "\"ecValue\":" + String(ecValue);
  json += "\"oxygenValue\":" + String(oxygenValue);

  json += "}";

  server.send(200, "application/json", json);
}

// Handle saving Wi-Fi credentials
void handleSaveWiFi() {
  ssid = server.arg("ssid");
  password = server.arg("password");
  server.send(200, "text/plain", "Wi-Fi credentials saved. Restarting...");
  delay(1000);
  ESP.restart();
}

// Handle saving threshold values
void handleSaveThresholds() {
  tempUpperLimit = server.arg("tempUpper").toFloat();
  tempLowerLimit = server.arg("tempLower").toFloat();
  humidityUpperLimit = server.arg("humidityUpper").toFloat();
  humidityLowerLimit = server.arg("humidityLower").toFloat();
  waterUpperLimit = server.arg("waterUpper").toFloat();
  waterLowerLimit = server.arg("waterLower").toFloat();
  pHUpperLimit = server.arg("pHUpper").toFloat();
  pHLowerLimit = server.arg("pHLower").toFloat();
  ecUpperLimit = server.arg("ecUpper").toFloat();
  ecLowerLimit = server.arg("ecLower").toFloat();
  oxygenUpperLimit = server.arg("oxygenUpper").toFloat();
  oxygenLowerLimit = server.arg("oxygenLower").toFloat();
  server.send(200, "text/plain", "Thresholds saved.");
}

// Handle toggling devices
void handleToggleDevice() {
  String device = server.arg("device");
  if (device == "waterPump") {
    pumpState = !pumpState;
    digitalWrite(RELAY_PUMP, pumpState ? HIGH : LOW);
    server.send(200, "text/plain", pumpState ? "ON" : "OFF");
  } else if (device == "fertilizerPump") {
    fertilizerState = !fertilizerState;
    digitalWrite(RELAY_FERTILIZER, fertilizerState ? HIGH : LOW);
    server.send(200, "text/plain", fertilizerState ? "ON" : "OFF");
  } else if (device == "mistGenerator") {
    mistState = !mistState;
    digitalWrite(RELAY_MIST, mistState ? HIGH : LOW);
    server.send(200, "text/plain", mistState ? "ON" : "OFF");
  } else if (device == "solenoidValve") {
    solenoidState = !solenoidState;
    digitalWrite(RELAY_SOLENOID, solenoidState ? HIGH : LOW);
    server.send(200, "text/plain", solenoidState ? "ON" : "OFF");
  } else if (device == "oxygenValve") {
    oxygenState = !oxygenState;
    digitalWrite(RELAY_OXYGEN_PUMP, oxygenState ? HIGH : LOW);
    server.send(200, "text/plain", oxygenState ? "ON" : "OFF");
  } else {
    server.send(400, "text/plain", "Invalid device");
  }
}

// Handle not found
void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

// Connect to Wi-Fi
void setupWiFi() {
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.println("Connecting to Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  Serial.println("Connected to Wi-Fi");
  Serial.println(WiFi.localIP());
}

// Update LCD display
void updateLCD() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  float waterLevel = measureWaterLevel();
  float pHValue = readPHSensor();
  float ecValue = readECSensor();
  float oxygenValue = readOxygenSensor();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print("%");

  lcd.setCursor(0, 2);
  lcd.print("Water: ");
  lcd.print(waterLevel);
  lcd.print("cm");

  lcd.print("Oxygen: ");
  lcd.print(oxygenValue);
  lcd.print("%");

  lcd.setCursor(0, 3);
  lcd.print("pH: ");
  lcd.print(pHValue);
  lcd.print(" EC: ");
  lcd.print(ecValue);
}

// Measure water level using HC-SR04
float measureWaterLevel() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.0343 / 2; // Convert to cm
  return distance;
}
//----------------------------
// Read pH sensor value
float readPHSensor() {
  int sensorValue = analogRead(PH_SENSOR_PIN);
  float voltage = (sensorValue / 4095.0) * 3.3;  // Convert to voltage (0-3.3V)  
  float slope = -5.7;  // Typical slope value for pH sensors
  float intercept = 16.8; // Intercept at pH 7
  float pHValue = slope * voltage + intercept;
  //float voltage = sensorValue * (3.3 / 4095.0); // Convert to voltage (ESP32 ADC resolution is 12-bit)
  //float pHValue = 7.0 - ((voltage - 2.5) / 0.18); // Calibration formula (adjust as needed)
  return pHValue;
}

//-------------------

// Read electroconductivity sensor value
float readECSensor() {
    int rawEC = analogRead(EC_SENSOR_PIN);
    float voltage = rawEC * (3.3 / 4095.0);
    // Convert voltage to raw EC value (example formula, adjust for your sensor)
    float rawECValue = voltage * calibrationFactor;
    // Apply temperature compensation
    float temperature = dht.readTemperature();   
    float compensatedEC = rawECValue / (1.0 + tempCoefficient * (temperature - 25.0));
    return compensatedEC;
}


// Read  Oxygen Sensor  value
float readOxygenSensor() {
    int rawOxygen = analogRead(OXYGEN_SENSOR_PIN);
    float voltage = rawOxygen * (vRef / adcResolution);

    // Convert voltage to dissolved oxygen (example formula, adjust for your sensor)
    float rawOxygenValue = voltage * calibrationFactor;

    // Apply temperature compensation
     float temperature = dht.readTemperature(); 
    float compensatedOxygen = rawOxygenValue * (1.0 + OxygentempCoefficient * (temperature - 25.0));

    return compensatedOxygen;
}


// Handle fertilization logic
void handleFertilization() {
  float ecValue = readECSensor();
  if (ecValue < ecLowerLimit) {
    digitalWrite(RELAY_FERTILIZER, HIGH); // Turn on fertilizer pump
    fertilizerState = true;
  } else if (ecValue > ecUpperLimit) {
    digitalWrite(RELAY_FERTILIZER, LOW); // Turn off fertilizer pump
    fertilizerState = false;
  }
}

 // Handle Water Intake Solenoid logic
void handleWaterSolenoid() {
  float waterLevel = measureWaterLevel();

  if (waterLevel < waterLowerLimit) {
    digitalWrite(RELAY_SOLENOID, HIGH); // Turn on Water Intake Solenoid
    solenoidState = true;
  } else if (waterLevel > waterUpperLimit) {
    digitalWrite(RELAY_SOLENOID, LOW); // Turn off Water Intake Solenoid
    solenoidState = false;
  }
}

  
  // Handle Water Pump logic
void handleWaterPump() {
  float waterLevel = measureWaterLevel();

  if (waterLevel < waterLowerLimit) {
    digitalWrite(RELAY_PUMP, HIGH); // Turn on Water pump
    pumpState = true;
  } else if (waterLevel > waterUpperLimit) {
    digitalWrite(RELAY_PUMP, LOW); // Turn off Water pump
    pumpState = false;
  }
}

 // Handle  Mist Pump logic
void handleMistPump() {
  float humidity = dht.readHumidity();
  if (humidity < humidityLowerLimit) {
    digitalWrite(RELAY_MIST, HIGH); // Turn on Mist pump
    mistState = true;
  } else if (humidity > humidityUpperLimit) {
    digitalWrite(RELAY_MIST, LOW); // Turn off Mist pump
    mistState = false;
  }
}
  
  // Handle Oxygen Pump logic
void handleOxygenPump() {
  float oxygenValue = readOxygenSensor();
  if (oxygenValue < oxygenLowerLimit) {
    digitalWrite(RELAY_OXYGEN_PUMP, HIGH); // Turn on Oxygen pump
    oxygenState = true;
  } else if (oxygenValue > oxygenUpperLimit) {
    digitalWrite(RELAY_OXYGEN_PUMP, LOW); // Turn off Oxygen pump
    oxygenState = false;
  }
}


