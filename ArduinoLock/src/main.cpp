#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include "./Keypad/Keypad.h"
#include "./LCD/display.h"
#include "./Servo/Servo.h"
#define RESET_PASSWORD_TIME 5000

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

// Password management
String correctPassword = "123";
const String emergencyPassword = "000";
const String adminPassword = "777";
String enteredPassword = "";
int attemptCount = 0;
bool stopAlarm = false;
bool messedUp = false;
bool resettingPassword = false;
bool policeAlarmActive = false;
unsigned long lastTimeKeyPressed = 0;
unsigned long lastTimeAlarm1000 = 0;
unsigned long lastTimeAlarm500 = 0;
unsigned long lastTimeResetPassword = 0;
unsigned long enteredAdminPassword = 0;

void accessGranted();
void accessDenied();
void resetPassword();
void handlePasswordReset(char key);
void handlePasswordEntry(char key);
void policeAlarm();
void handlePoliceAlarmKeypad(char key);

void setup()
{
  Serial.begin(9600);
  myServo.attach(servoPin);
  keypad.initialize();
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, HIGH); // Ensure buzzer is off initially
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  lcd.initialize();
}

void loop()
{
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
        digitalWrite(redPin, LOW);
        digitalWrite(greenPin, LOW);
        digitalWrite(bluePin, LOW);
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
    // return;
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
    // Serial.println(static_cast<unsigned long>(lastTimeKeyPressed));
    // Serial.println("millis: " + String(millis()));
    enteredPassword = "";
    lastTimeKeyPressed = millis();
    resettingPassword = true;
    lcd.clear();
    lcd.printMessage(ONE_LINE_MESSAGE, "Enter Admin Password:");
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, HIGH);
    digitalWrite(bluePin, LOW);
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
        digitalWrite(redPin, LOW);
        digitalWrite(bluePin, LOW);
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
