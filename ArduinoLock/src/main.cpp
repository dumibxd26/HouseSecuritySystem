#include <Arduino.h>

int counter = 1;

void setup()
{
  Serial.begin(9600);
}

void loop()
{
  // Send initial message to ESP32-CAM
  Serial.println("message" + String(counter));
  delay(1000); // Delay before sending the next message
  counter++;

  // Check if data is available from ESP32-CAM
  if (Serial.available() > 0)
  {
    String message = Serial.readStringUntil('\n');
    // Print the received message
    Serial.println("Received: " + message);
  }

  // every 5 seconds send a message "start"
  if (counter % 5 == 0 && counter % 10 != 0)
  {
    Serial.println("start");
  }

  // every 10 seconds send a message "stop"
  if (counter % 10 == 0)
  {
    Serial.println("stop");
  }
}
