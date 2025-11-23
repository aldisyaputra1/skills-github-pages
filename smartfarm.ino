#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

// Konfigurasi WiFi
const char* ssid = "NAMA_WIFI_ANDA";
const char* password = "PASSWORD_WIFI_ANDA";

// Konfigurasi Server
const char* serverURL = "http://your-server.com/api/sensor-data";

// Konfigurasi Sensor
#define DHT_PIN 4           // Pin untuk DHT11/DHT22
#define DHT_TYPE DHT22      // Tipe DHT: DHT11 atau DHT22
#define SOIL_MOISTURE_PIN 34 // Pin untuk sensor kelembaban tanah
#define LIGHT_SENSOR_PIN 35  // Pin untuk sensor intensitas cahaya

// Inisialisasi sensor DHT
DHT dht(DHT_PIN, DHT_TYPE);

// Variabel untuk penyimpanan data
float temperature = 0;
float humidity = 0;
int soilMoisture = 0;
int lightIntensity = 0;

// Timer untuk pengiriman data
unsigned long previousMillis = 0;
const long interval = 30000; // Interval 30 detik

void setup() {
  Serial.begin(115200);
  
  // Inisialisasi sensor
  dht.begin();
  pinMode(SOIL_MOISTURE_PIN, INPUT);
  pinMode(LIGHT_SENSOR_PIN, INPUT);
  
  // Koneksi WiFi
  connectToWiFi();
  
  Serial.println("Smart Farm Monitoring System Started");
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Baca sensor setiap interval
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    readSensors();
    sendDataToServer();
    displayData();
  }
  
  delay(1000);
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println("\nConnected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void readSensors() {
  // Baca suhu dan kelembaban udara
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  
  // Baca kelembaban tanah (analog read)
  soilMoisture = analogRead(SOIL_MOISTURE_PIN);
  soilMoisture = map(soilMoisture, 0, 4095, 0, 100); // Konversi ke persentase
  
  // Baca intensitas cahaya
  lightIntensity = analogRead(LIGHT_SENSOR_PIN);
  lightIntensity = map(lightIntensity, 0, 4095, 0, 100); // Konversi ke persentase
  
  // Validasi pembacaan sensor DHT
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    temperature = 0;
    humidity = 0;
  }
}

void sendDataToServer() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    // Buat JSON data
    DynamicJsonDocument doc(1024);
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["soil_moisture"] = soilMoisture;
    doc["light_intensity"] = lightIntensity;
    doc["device_id"] = "ESP32_FARM_001";
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    // Konfigurasi HTTP request
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");
    
    // Kirim data
    int httpResponseCode = http.POST(jsonString);
    
    if (httpResponseCode > 0) {
      Serial.print("Data sent successfully. Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error sending data. Response code: ");
      Serial.println(httpResponseCode);
    }
    
    http.end();
  } else {
    Serial.println("WiFi disconnected. Reconnecting...");
    connectToWiFi();
  }
}

void displayData() {
  Serial.println("=== Sensor Data ===");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
  
  Serial.print("Soil Moisture: ");
  Serial.print(soilMoisture);
  Serial.println(" %");
  
  Serial.print("Light Intensity: ");
  Serial.print(lightIntensity);
  Serial.println(" %");
  Serial.println("====================");
}