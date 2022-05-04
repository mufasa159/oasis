// Author: Mufasa A.
// Email: hello@mufasa.cc
// Description: Program for auto-watering system for indoor plants

#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

float humidity;
float temperature;
float moisture;

float MAX_MOIST = 300.00;    // Higher ? soil is dry
float MIN_HUMIDITY = 45.00;  // Lower  ? lack of water on air

int water_pump = 6;          // D6 Pin

void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(water_pump, OUTPUT);
}

void loop() {
  moisture = analogRead(A1);
  humidity = dht.readHumidity();
  temperature = (dht.readTemperature() * 1.8) + 32;
  
  if (moisture > MAX_MOIST && humidity < MIN_HUMIDITY) {
    digitalWrite(water_pump, HIGH);
    delay(5000);
    digitalWrite(water_pump, LOW);
    delay(10000);
  }

  Serial.print("Hmdt: ");
  Serial.print(humidity);
  Serial.print(" %   Temp: ");
  Serial.print(temperature);
  Serial.print("ºF   Mstr: ");
  Serial.println(moisture);
    
  delay(5000);
    
}
