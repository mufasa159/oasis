// Author: Mufasa A.
// Email: hello@mufasa.cc
// Description: An auto irrigation system for indoor plants


#include <DHT.h>
#define PIN_BUTTON 2
#define PIN_DHT22 3
#define PIN_LED 4
#define PIN_PUMP 6
#define PIN_MOIST A1

DHT dht(PIN_DHT22, DHT22);

// ----------- User Settings Variables -------------

boolean displayFahrenheit = false;

enum Moisture {
  VERY_WET = 150,
  WET = 200,
  NORMAL = 350,
  DRY = 650,
  VERY_DRY = 800
};

// -------------------------------------------------

float humidity;
float temperature;
float moisture;

const int DISCONNECTED = VERY_DRY; // Higher ? sensor possibly disconnected
Moisture MAX_MOIST = NORMAL;       // Higher ? soil is dry
const int TOO_DRY = DRY;           // Higher ? too dry to consider humidity
const float MIN_HUMIDITY = 40.00;  // Lower  ? lack of water on air

boolean pumpOn = false;
int currentPumpButtonValue = 0;
int previousPumpButtonValue = 1;
boolean pumpDisconnected = false;

long previousTime_01 = 0;
long previousTime_02 = 0;
long previousTime_03 = 0;

void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(PIN_PUMP, OUTPUT);
  pinMode(PIN_BUTTON, INPUT);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_BUTTON, HIGH);
}

void loop() {

  // Direct readings from sensors
  moisture = analogRead(PIN_MOIST);
  humidity = dht.readHumidity();
  temperature = toFahrenheit(dht.readTemperature());


  // Water pump power control via push-button
  digitalWrite(PIN_PUMP, LOW);
  currentPumpButtonValue = digitalRead(PIN_BUTTON);
  if (currentPumpButtonValue == 0) {
    previousPumpButtonValue = !previousPumpButtonValue;
    if (previousPumpButtonValue == 0) {
      pumpDisconnected = true;
    } else {
      pumpDisconnected = false;
    }
    while (currentPumpButtonValue == 0) {
      currentPumpButtonValue = digitalRead(PIN_BUTTON);
    }
  }

  
  // Timer for causing delay
  unsigned long currentTime = millis();


  // Pump LED Control
  if (pumpDisconnected) {
    digitalWrite(PIN_LED, LOW);
  } else {
    digitalWrite(PIN_LED, HIGH);
  }


  // Main logic for watering plants
  if (currentTime - previousTime_03 >= 3000) {
    if (!pumpDisconnected) {
      if (moisture > DISCONNECTED) {
        if (currentTime - previousTime_01 >= 3500) {
          Serial.println("Moist Sensor Disonnected");
          previousTime_01 = currentTime;
        }
      } else if (moisture > TOO_DRY) {
        if (currentTime - previousTime_01 >= 3500) {
          pumpOn = false;
          previousTime_01 = currentTime;
        } else {
          pumpOn = true;
        }
      } else if (moisture > MAX_MOIST && humidity < MIN_HUMIDITY) {
        if (currentTime - previousTime_01 >= 3500) {
          pumpOn = false;
          previousTime_01 = currentTime;
        } else {
          pumpOn = true;
        }
      }
    }
    Serial.print("Hmdt: ");
    Serial.print(humidity);
    Serial.print(" %   Temp: ");
    Serial.print(temperature);
    Serial.print("ºF   Mstr: ");
    Serial.print(moisture);
    Serial.print("   PumpDiscnt: ");
    Serial.println(pumpDisconnected);
    previousTime_03 = currentTime;
  }
  digitalWrite(PIN_PUMP, pumpOn);
}


// Convert Celsius temperature to Fahrenheit
float toFahrenheit(float celsiusValue) {
  celsiusValue = (celsiusValue * 1.8) + 32;
  return celsiusValue;
}
