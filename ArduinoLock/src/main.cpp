#include <Arduino.h>
#include <Wire.h>
#include "./Keypad/Keypad.h"
#include "./LCD/display.h"
#include "./Servo/Servo.h"
#define RESET_PASSOWORD_TIME 5000

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
int lastTimeKeyPressed = 0;
int lastTimeAlarm1000 = 0;
int lastTimeAlarm500 = 0;
int lastTimeMessedUpKeyPressed = 0;
int enteredAdminPassword = 0;

// void policeAlarm();
void accessGranted();
void accessDenied();

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
  char key = keypad.getKey();
  int currentTime = millis();
  if (key != '\0' && currentTime - lastTimeKeyPressed > 250 && !messedUp)
  {
    if (key != keypad.getLastKeyPressed() && !resettingPassword)
    {
      keypad.setLastKeyPressed(key);
      lastTimeKeyPressed = currentTime;
      // // set this here for the following case:
      // // when the user enters the emergency code, the first time, te last key pressed for the password
      // // is considered as the first key of the emergency code, if we do this it will have the correct debounce
      // lastTimeMessedUpKeyPressed = currentTime;
      enteredPassword += key;
      lcd.clear();
      lcd.printMessage(ONE_LINE_MESSAGE, "Enter Password:");
      lcd.printMessage(ONE_LINE_MESSAGE, enteredPassword.c_str());

      if (enteredPassword.length() == 3)
      {
        if (enteredPassword == correctPassword)
        {
          accessGranted();
        }
        else
        {
          accessDenied();
          // to prevent the error mentioned above which's solutions was not that
          key = '\0';
        }
        enteredPassword = ""; // Reset password entry
      }
    }
    else if (key == 'D' && (resettingPassword || currentTime - lastTimeKeyPressed > RESET_PASSOWORD_TIME))
    {
      if (!resettingPassword)
      {
        resettingPassword = true;
        enteredPassword = "";
      }

      enteredPassword += key;
      lcd.clear();
      if (enteredAdminPassword == 0)
      {
        lcd.printMessage(ONE_LINE_MESSAGE, "Enter Admin Password:");
      }
      else
      {
        lcd.printMessage(ONE_LINE_MESSAGE, "Enter New Password:");
      }
      digitalWrite(redPin, HIGH);
      digitalWrite(greenPin, HIGH);
      digitalWrite(bluePin, LOW);

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
            lcd.printMessage(ONE_LINE_MESSAGE, enteredPassword.c_str());
          }
          else
          {
            lcd.clear();
            lcd.printMessage(ONE_LINE_MESSAGE, "Wrong Admin Password");
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
          digitalWrite(redPin, LOW);
          digitalWrite(greenPin, LOW);
          digitalWrite(bluePin, LOW);
        }
      }
      delay(250);
    }
  }

  if (messedUp)
  {
    if (currentTime - lastTimeAlarm1000 > 300)
    {
      lastTimeAlarm1000 = currentTime;
      tone(buzzerPin, 1000, 300);
    }
    if (currentTime - lastTimeAlarm500 > 600)
    {
      lastTimeAlarm500 = currentTime;
      tone(buzzerPin, 500, 300);
    }

    if (key)
    {
      int currentTime = millis();
      if (currentTime - lastTimeMessedUpKeyPressed > 250 && key != keypad.getLastKeyPressed())
      {
        Serial.println("Key: " + String(key));
        lastTimeMessedUpKeyPressed = currentTime;
        enteredPassword += key;

        if (enteredPassword.length() == 3)
        {
          if (enteredPassword == emergencyPassword)
          {
            noTone(buzzerPin);
            digitalWrite(buzzerPin, LOW); // Turn off buzzer
            messedUp = false;
            accessGranted();
          }
          enteredPassword = ""; // Reset password entry
        }
      }
    }
  }
}

void accessGranted()
{
  lcd.clear();
  lcd.printMessage(ONE_LINE_MESSAGE, "Access Granted");

  digitalWrite(greenPin, HIGH); // Turn on green LED
  myServo.write(0);             // Rotate servo to open

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

  myServo.write(90);           // Rotate servo back to closed
  digitalWrite(greenPin, LOW); // Turn off green LED

  // rotate the servo motor back to the initial position
  myServo.write(180);
  delay(300 * 3 + 600);
  myServo.write(90);
}

void accessDenied()
{
  attemptCount++;
  lcd.clear();
  lcd.printMessage(ONE_LINE_MESSAGE, "Access Denied");
  digitalWrite(redPin, HIGH); // Turn on red LED
  tone(buzzerPin, 1000, 100); // Play error sound
  delay(100);
  noTone(buzzerPin);
  digitalWrite(buzzerPin, HIGH); // Turn off buzzer
  digitalWrite(redPin, LOW);     // Turn off red LED
  delay(1000);

  if (attemptCount >= 3)
  {
    lcd.clear();
    lcd.printMessage(ONE_LINE_MESSAGE, "Call 911");
    messedUp = true;
    attemptCount = 0; // Reset attempt count after alarm
  }
  else
  {
    lcd.printMessage(ONE_LINE_MESSAGE, "Enter Password:");
  }
}

bool checkEmergencyPassword()
{
  if (enteredPassword == emergencyPassword)
  {
    lcd.clear();
    lcd.printMessage(ONE_LINE_MESSAGE, "Enter Emergency Code");
    accessGranted();
    return true;
  }
  return false;
}
