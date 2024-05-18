#include <Arduino.h>
#include <WiFi.h>

const int ledPin = 4; // Built-in LED pin

void setup()
{
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
}

void loop()
{
  // Check if data is available from Arduino Uno
  if (Serial.available() > 0)
  {
    String message = Serial.readStringUntil('\n');
    // Check if message is "start" to turn on the flash
    if (message.startsWith("start"))
    {
      digitalWrite(ledPin, HIGH); // Turn on the flash
      // Serial.println("Flash turned on");
    }
    // Check if message is "stop" to turn off the flash
    else if (message.startsWith("stop"))
    {
      digitalWrite(ledPin, LOW); // Turn off the flash
      // Serial.println("Flash turned off");
    }
    // Echo the received message back to Arduino Uno
    else
    {
      if (!message.startsWith("Received"))
        Serial.println("Received message: " + message);
    }
  }
}
