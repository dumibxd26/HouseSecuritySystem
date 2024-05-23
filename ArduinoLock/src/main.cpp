#include <Arduino.h>
#include <Wire.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include "./Keypad/Keypad.h"
#include "./LCD/display.h"
#include "./Servo/Servo.h"

#define RESET_PASSWORD_TIME 5000
#define ACTIVATION_DISTANCE 10
#define DEACTIVATION_TIME 60000 // 60 seconds

// Keypad setup
const int numRows = 4; // four rows
const int numCols = 4; // four columns

const int rowPins[numRows] = {6, 7, 8, 9}; // row pins connected to R1, R2, R3, R4
const int colPins[numCols] = {2, 3, 4, 5}; // column pins connected to C1, C2, C3, C4

const char *keys[] = {
    "123A",
    "456B",
    "789C",
    "*0#D"};

Keypad keypad(numRows, numCols, rowPins, colPins, keys);

// RGB LED pins
const int redPin = 10;
const int greenPin = 11;
const int bluePin = 12;

// Active Buzzer Pin
const int buzzerPin = 13;

// I2C LCD Address and Initialization
Display lcd;

// Servo motor configuration
Servo myServo;
const int servoPin = A0;

// Ultrasonic Sensor pins
const int trigPin = A1;
const int echoPin = A2;
const int esp32CamPin = A3;
unsigned long duration;
int distance;
const int distanceThreshold = 10; // Set distance threshold (in cm) for detection
bool distanceDetected = false;
bool previousDistanceDetected = false; // Track previous detection state

// Password management
String correctPassword = "123";
const String emergencyPassword = "000";
const String adminPassword = "777";
String enteredPassword = "";
int attemptCount = 0;
bool resettingPassword = false;
bool policeAlarmActive = false;
unsigned long lastTimeKeyPressed = 0;
unsigned long lastTimeAlarm1000 = 0;
unsigned long lastTimeAlarm500 = 0;
unsigned long lastDistanceCheck = 0;
unsigned long enteredAdminPassword = 0;
unsigned long activationTime = 0; // Time when the system was activated
bool systemActive = false;        // Indicates whether the system is active

void accessGranted();
void accessDenied();
void resetPassword();
void handlePasswordReset(char key);
void handlePasswordEntry(char key);
void policeAlarm();
void handlePoliceAlarmKeypad(char key);
void checkDistance();
void setLedColor(bool red, bool green, bool blue);
void deactivateSystem();
void enterSleepMode();
void wakeUp();

void setup()
{
  Serial.begin(9600); // Initialize serial communication with ESP32-CAM
  myServo.attach(servoPin);
  keypad.initialize();
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, HIGH); // Ensure buzzer is off initially
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  lcd.initialize();

  // Ultrasonic Sensor setup
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(esp32CamPin, OUTPUT);

  digitalWrite(echoPin, HIGH);

  digitalWrite(esp32CamPin, LOW);

  // Initially deactivate the system
  deactivateSystem();
}

void loop()
{
  // Check distance periodically
  if (millis() - lastDistanceCheck > 500)
  {
    lastDistanceCheck = millis();
    checkDistance();
  }

  // If the system is not active, do not proceed further
  if (!systemActive)
  {
    return;
  }

  if (Serial.available() > 0)
  {
    String message = Serial.readString();
    // Serial.println(message);
    if (message.startsWith("Aon"))
    {
      lcd.clear();
      lcd.printMessage(ONE_LINE_MESSAGE, "Calling 911");
      policeAlarmActive = true;
      enteredPassword = "";
      attemptCount = 0;
    }
    else if (message.startsWith("Aoff"))
    {
      policeAlarmActive = false;
      noTone(buzzerPin);
      digitalWrite(buzzerPin, HIGH);
      setLedColor(false, false, false);
      lcd.clear();
      lcd.printMessage(ONE_LINE_MESSAGE, "Alarm Deactivated!");
      delay(1000);
      lcd.clear();
      lcd.printMessage(ONE_LINE_MESSAGE, "Enter Password:");
      enteredPassword = "";
    }
    else if (message.startsWith("newPassword"))
    {
      // noticed when writing to the lcd the password
      correctPassword = message.substring(message.indexOf("[") + 2, message.indexOf("]") - 1);
      lcd.clear();
      // lcd.printMessage(ONE_LINE_MESSAGE, "Password Changed");
      lcd.printMessage(TWO_LINE_MESSAGE, "Password Changed", correctPassword.c_str());
      enteredPassword = "";
      resettingPassword = false;
      enteredAdminPassword = 0;
      attemptCount = 0;
      setLedColor(false, false, false);
      delay(500);
      lcd.clear();
      lcd.printMessage(ONE_LINE_MESSAGE, "Enter Password:");
    }
    else if (message.startsWith("allowAccess"))
    {
      accessGranted();
    }
  }

  // Handle police alarm if active
  if (policeAlarmActive)
  {
    policeAlarm();
    char key = keypad.getKey();
    if (key != '\0' && millis() - lastTimeKeyPressed >= 250)
    {
      handlePoliceAlarmKeypad(key); // Handle admin password entry during alarm
    }
    return;
  }

  // Handle keypad input
  char key = keypad.getKey();

  if (key == '\0')
  {
    return;
  }

  if (millis() - lastTimeKeyPressed < 250)
  {
    return;
  }

  if (resettingPassword)
  {
    handlePasswordReset(key);
  }
  else
  {
    handlePasswordEntry(key);
  }

  // Deactivate the system if the door is not opened within 60 seconds
  if (millis() - activationTime >= DEACTIVATION_TIME && !policeAlarmActive && systemActive && !resettingPassword && !distanceDetected)
  {
    deactivateSystem();
  }
}

void handlePasswordReset(char key)
{
  if (key == 'D')
  {
    enteredPassword = "";
    if (enteredAdminPassword == 0)
    {
      lcd.clear();
      lcd.printMessage(ONE_LINE_MESSAGE, "Enter Admin Password:");
    }
    else
    {
      lcd.clear();
      lcd.printMessage(ONE_LINE_MESSAGE, "Enter New Password:");
    }
    return;
  }

  if (key != keypad.getLastKeyPressed())
  {
    keypad.setLastKeyPressed(key);
    lastTimeKeyPressed = millis();
    enteredPassword += key;

    if (enteredAdminPassword == 0)
    {
      lcd.printMessage(TWO_LINE_MESSAGE, "Enter Admin Password:", enteredPassword.c_str());
    }
    else
    {
      lcd.printMessage(TWO_LINE_MESSAGE, "Enter New Password:", enteredPassword.c_str());
    }

    if (enteredPassword.length() == 3)
    {
      if (enteredAdminPassword == 0)
      {
        if (enteredPassword == adminPassword)
        {
          enteredAdminPassword = 1;
          enteredPassword = "";
          lcd.clear();
          lcd.printMessage(ONE_LINE_MESSAGE, "Enter New Password:");
        }
        else
        {
          lcd.clear();
          lcd.printMessage(ONE_LINE_MESSAGE, "Wrong Admin Password");
          delay(500);
          lcd.clear();
          lcd.printMessage(ONE_LINE_MESSAGE, "Enter Admin Password:");
          enteredPassword = "";
        }
      }
      else
      {
        correctPassword = enteredPassword;
        lcd.clear();
        lcd.printMessage(ONE_LINE_MESSAGE, "Password Changed");
        enteredPassword = "";
        resettingPassword = false;
        enteredAdminPassword = 0;
        setLedColor(false, false, false);
        delay(500);
        lcd.clear();
        lcd.printMessage(ONE_LINE_MESSAGE, "Enter Password:");
        attemptCount = 0;
      }
    }
    delay(250);
  }
}

void handlePasswordEntry(char key)
{
  if (key == 'D' && keypad.getLastKeyPressed() != 'D')
  {
    enteredPassword = "";
    lcd.clear();
    lcd.printMessage(ONE_LINE_MESSAGE, "Enter Password:");
    lastTimeKeyPressed = millis();
    keypad.setLastKeyPressed(key);
  }
  if (key != keypad.getLastKeyPressed() && key != 'D')
  {
    keypad.setLastKeyPressed(key);
    lastTimeKeyPressed = millis();
    enteredPassword += key;
    lcd.clear();
    lcd.printMessage(TWO_LINE_MESSAGE, "Enter Password:", enteredPassword.c_str());

    if (enteredPassword.length() == 3)
    {
      if (enteredPassword == correctPassword)
      {
        accessGranted();
      }
      else
      {
        accessDenied();
      }
      enteredPassword = ""; // Reset password entry
    }
  }
  else if (enteredPassword.length() < 2 && key == 'D' && keypad.getLastKeyPressed() == 'D' && millis() - lastTimeKeyPressed > RESET_PASSWORD_TIME)
  {
    enteredPassword = "";
    lastTimeKeyPressed = millis();
    resettingPassword = true;
    lcd.clear();
    lcd.printMessage(ONE_LINE_MESSAGE, "Enter Admin Password:");
    setLedColor(true, true, false);
  }
}

void handlePoliceAlarmKeypad(char key)
{
  if (key == 'D')
  {
    enteredPassword = "";
    lastTimeKeyPressed = millis();
    return;
  }

  if (key != keypad.getLastKeyPressed())
  {
    keypad.setLastKeyPressed(key);
    lastTimeKeyPressed = millis();
    enteredPassword += key;

    if (enteredPassword.length() == 3)
    {
      if (enteredPassword == adminPassword)
      {
        policeAlarmActive = false;
        noTone(buzzerPin);
        digitalWrite(buzzerPin, HIGH);
        setLedColor(false, false, false);
        lcd.clear();
        lcd.printMessage(ONE_LINE_MESSAGE, "Alarm Deactivated!");
        delay(1000);
        lcd.clear();
        lcd.printMessage(ONE_LINE_MESSAGE, "Enter Password:");
        enteredPassword = "";
      }
      else
      {
        enteredPassword = "";
      }
    }
    delay(250);
  }
}

void accessGranted()
{
  lcd.clear();
  lcd.printMessage(ONE_LINE_MESSAGE, "Access Granted");

  digitalWrite(greenPin, HIGH);
  myServo.write(0);

  tone(buzzerPin, 800, 300);
  delay(300);
  noTone(buzzerPin);
  tone(buzzerPin, 1000, 300);
  delay(300);
  noTone(buzzerPin);
  tone(buzzerPin, 1200, 300);
  delay(300);
  noTone(buzzerPin);
  tone(buzzerPin, 1400, 600);
  delay(600);
  noTone(buzzerPin);
  digitalWrite(buzzerPin, HIGH);

  myServo.write(90);
  delay(5000);
  digitalWrite(greenPin, LOW);

  myServo.write(180);
  delay(300 * 3 + 600);
  myServo.write(90);

  lcd.clear();
  lcd.printMessage(ONE_LINE_MESSAGE, "Enter Password:");

  attemptCount = 0;
  enteredPassword = "";
}

void accessDenied()
{
  attemptCount++;
  lcd.clear();
  lcd.printMessage(ONE_LINE_MESSAGE, "Access Denied");
  digitalWrite(redPin, HIGH);
  tone(buzzerPin, 1000, 100);
  delay(100);
  noTone(buzzerPin);
  digitalWrite(buzzerPin, HIGH);
  digitalWrite(redPin, LOW);

  if (attemptCount >= 3)
  {
    lcd.clear();
    lcd.printMessage(ONE_LINE_MESSAGE, "Calling 911");
    policeAlarmActive = true;
    enteredPassword = "";
    attemptCount = 0;
  }
  else
  {
    delay(1000);
    lcd.printMessage(ONE_LINE_MESSAGE, "Enter Password:");
  }
}

void policeAlarm()
{
  unsigned long currentMillis = millis();

  // buzzer synced with red and blue LEDs :D
  if (currentMillis - lastTimeAlarm1000 > 300)
  {
    lastTimeAlarm1000 = currentMillis;
    static bool toggleSiren = false;
    toggleSiren = !toggleSiren;
    if (toggleSiren)
    {
      tone(buzzerPin, 1000); // Tone 1
    }
    else
    {
      tone(buzzerPin, 1200); // Tone 2
    }
  }

  // Toggle red and blue LEDs
  if (currentMillis - lastTimeAlarm500 > 300)
  {
    lastTimeAlarm500 = currentMillis;
    static bool toggleLED = false;
    toggleLED = !toggleLED;
    if (toggleLED)
    {
      digitalWrite(redPin, HIGH);
      digitalWrite(bluePin, LOW);
    }
    else
    {
      digitalWrite(redPin, LOW);
      digitalWrite(bluePin, HIGH);
    }
  }
}

void checkDistance()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  if (distance > 0 && distance <= ACTIVATION_DISTANCE)
  {
    if (!systemActive)
    {
      systemActive = true;
      activationTime = millis();
      lcd.clear();
      lcd.printMessage(ONE_LINE_MESSAGE, "Enter Password:");
      lcd.backlight(); // Turn on the LCD backlight
    }
  }
  else if (systemActive && (millis() - activationTime >= DEACTIVATION_TIME) && !policeAlarmActive)
  {
    deactivateSystem();
  }

  if (distance > 0 && distance <= distanceThreshold)
  {
    distanceDetected = true;
    if (!previousDistanceDetected) // If it was not previously detected
    {
      // Serial.println("start"); // Send message to ESP32-CAM
      digitalWrite(esp32CamPin, HIGH);
      delay(1000);
      digitalWrite(esp32CamPin, LOW);
    }
  }
  else
  {
    distanceDetected = false;
    if (previousDistanceDetected) // If it was previously detected
    {
      Serial.println("stop"); // Send message to ESP32-CAM
    }
  }
  previousDistanceDetected = distanceDetected;
}

void setLedColor(bool red, bool green, bool blue)
{
  digitalWrite(redPin, red ? HIGH : LOW);
  digitalWrite(greenPin, green ? HIGH : LOW);
  digitalWrite(bluePin, blue ? HIGH : LOW);
}

void deactivateSystem()
{
  systemActive = false;

  // Turn off LCD backlight
  lcd.noBacklight();

  // Optionally turn off all peripherals or set them to a low-power state
  setLedColor(false, false, false);
  myServo.write(90);
  noTone(buzzerPin);
  digitalWrite(buzzerPin, HIGH);
  keypad.setLastKeyPressed('\0'); // Reset keypad state

  // Enter sleep mode
  enterSleepMode();
}

void enterSleepMode()
{
  set_sleep_mode(SLEEP_MODE_IDLE); // Set sleep mode to power down
  sleep_enable();                  // Enable sleep mode
  sleep_cpu();                     // Put the device to sleep
}

void wakeUp()
{
  sleep_disable(); // Disable sleep mode
}
