#include <Arduino.h>

const int soundSensorPin = A0; // Analog pin the sound sensor is connected to
const int threshold = 500;     // Adjust this threshold value according to your environment

void setup()
{
  Serial.begin(9600); // Initialize serial communication
}

void loop()
{
  int soundValue = analogRead(soundSensorPin); // Read the analog value from the sound sensor
                                               // Print the sound value to Serial Monitor
  if (soundValue > threshold)
  {                                    // If sound level is above the threshold
    Serial.println("Sound detected!"); // Print message to Serial Monitor
    delay(100);                        // Optional delay to avoid multiple detections within a short time
  }
}
