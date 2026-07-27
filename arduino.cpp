#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <FirebaseESP32.h>

#define PH_SENSOR_PIN 34       // pH sensor connected to GPIO 34 (ADC)
#define TDS_SENSOR_PIN 35      // TDS sensor connected to GPIO 35 (ADC)
#define TURBIDITY_SENSOR_PIN 32 // Turbidity sensor connected to GPIO 32 (ADC)
#define ACID_LEVEL_SENSOR_PIN 33 // Water level sensor connected to GPIO 33 (ADC)
#define SOLENOID_VALVE_PIN 25 // GPIO pin for solenoid valve control
#define FLOW_SENSOR_PIN 26     // Water flow sensor connected to GPIO 26 (Digital)

// Calibration constants for sensors
#define PH_CALIBRATION_OFFSET 0.0
#define TDS_CALIBRATION_FACTOR 0.5
#define ADC_RESOLUTION 4096.0
#define ESP32_VOLTAGE 3.3
#define FIREBASE_HOST "https://water-level-3f444-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "AIzaSyC0ZnLC-54jZ3vU0goFgtF_8tg_Oy7bVmY"  // Replace with your Firebase secret or token
#define WIFI_SSID "Redmi K50i"                // Replace with your WiFi SSID
#define WIFI_PASSWORD "kalaikumaru"  

FirebaseData firebaseData;
FirebaseConfig config;
LiquidCrystal_I2C lcd(0x27, 16, 2);

volatile int pulseCount = 0;
float flowRate = 0.0;
float totalLiters = 0.0;
unsigned long lastFlowTime = 0;

void IRAM_ATTR pulseCounter() {
  pulseCount++;
}

void sendToFirebase(float phValue, float tdsValue, float turbidity, float acidLevelPercentage, float flowRate, float totalLiters) {
Firebase.setFloat(firebaseData, "/WaterQualityData/phValue", phValue);
Firebase.setFloat(firebaseData, "/WaterQualityData/tdsValue", tdsValue);
Firebase.setFloat(firebaseData, "/WaterQualityData/turbidity", turbidity);
Firebase.setFloat(firebaseData, "/WaterQualityData/acidLevelPercentage", acidLevelPercentage);

Firebase.setFloat(firebaseData, "/WaterFlowData/flowRate", flowRate);
Firebase.setFloat(firebaseData, "/WaterFlowData/totalLiters", totalLiters);
}
void setup() {
  Serial.begin(115200); // Start serial communication
  analogReadResolution(12); // Set ADC resolution to 12-bit

  // Firebase Configuration Setup
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  // Set Firebase configuration
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;

  // Initialize Firebase
  Firebase.begin(&config, NULL); // Pass NULL for FirebaseAuth if not using it
  Firebase.reconnectWiFi(true);

  // Check if the connection to Firebase is successful
  if (Firebase.ready()) {
    Serial.println("Connected to Firebase");
  } else {
    Serial.println("Failed to connect to Firebase");
  }

  // Initialize the LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  delay(1000);

  pinMode(SOLENOID_VALVE_PIN, OUTPUT); // Set solenoid valve pin as output
  digitalWrite(SOLENOID_VALVE_PIN, LOW); // Initialize the valve as OFF

  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP); // Initialize flow sensor pin
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, RISING);
}

void loop() {
  // Read sensor values
  float phValue = readPH(PH_SENSOR_PIN);
  float tdsValue = readTDS(TDS_SENSOR_PIN);
  float turbidity = readTurbidity(TURBIDITY_SENSOR_PIN);
  float acidLevelPercentage = readAcidLevel(ACID_LEVEL_SENSOR_PIN);

  // Calculate flow rate and total liters
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - lastFlowTime;

  if (elapsedTime > 1000) {
    flowRate = (pulseCount / 7.5); // Flow rate in liters per minute
    totalLiters += (flowRate / 60.0); // Total liters
    pulseCount = 0;
    lastFlowTime = currentTime;
  }

  // Display results on Serial Monitor
  Serial.println("=========================");
  Serial.print("pH Level: ");
  Serial.println(phValue);

  Serial.print("TDS Level: ");
  Serial.print(tdsValue);
  Serial.println(" ppm");

  Serial.print("Turbidity Level: ");
  Serial.print(turbidity);
  Serial.println(" NTU");

  Serial.print("Acid Level: ");
  Serial.print(acidLevelPercentage);
  Serial.println(" %");

  Serial.print("Flow Rate: ");
  Serial.print(flowRate);
  Serial.println(" L/min");

  Serial.print("Total Liters: ");
  Serial.println(totalLiters);
  Serial.println("=========================");

  // Display results on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("pH:");
  lcd.print(phValue, 1);
  lcd.print(" TDS:");
  lcd.print(tdsValue, 1);

  lcd.setCursor(0, 1);
  lcd.print("Turb:");
  lcd.print(turbidity, 1);
  lcd.print(" Flow:");
  lcd.print(flowRate, 1);

  // Send data to Firebase
  sendToFirebase(phValue, tdsValue, turbidity, acidLevelPercentage, flowRate, totalLiters);

  // Check the solenoid valve state from Firebase
  if (Firebase.getInt(firebaseData, "/Control/SolenoidValve")) {
    int valveState = firebaseData.intData();
    if (valveState == 1) {
      digitalWrite(SOLENOID_VALVE_PIN, HIGH); // Turn ON the solenoid valve
    } else {
      digitalWrite(SOLENOID_VALVE_PIN, LOW); // Turn OFF the solenoid valve
    }
  } else {
    Serial.print("Failed to read solenoid valve state: ");
    Serial.println(firebaseData.errorReason());
  }

  delay(2000); // Wait for 2 seconds before the next iteration
}

// Function to read pH level
float readPH(int pin) {
  int adcValue = analogRead(pin);
  float voltage = adcValue * (ESP32_VOLTAGE / ADC_RESOLUTION);
  float ph = 3.5 * voltage + PH_CALIBRATION_OFFSET; // Example calibration equation
  return ph;
}

// Function to read TDS level
float readTDS(int pin) {
  int adcValue = analogRead(pin);
  float voltage = adcValue * (ESP32_VOLTAGE / ADC_RESOLUTION);
  float tds = voltage / TDS_CALIBRATION_FACTOR; // Example conversion
  return tds;
}

// Function to read turbidity level
float readTurbidity(int pin) {
  int adcValue = analogRead(pin);
  float voltage = adcValue * (ESP32_VOLTAGE / ADC_RESOLUTION);
  float turbidity = (voltage - 2.5) * -1120.4 + 536.7; // Example calibration equation
  return turbidity;
}

// Function to read acid level percentage
float readAcidLevel(int pin) {
  int adcValue = analogRead(pin);
  float voltage = adcValue * (ESP32_VOLTAGE / ADC_RESOLUTION);
  float levelPercentage = (voltage / ESP32_VOLTAGE) * 100.0; // Assuming full voltage equals 100%
  return levelPercentage;
}