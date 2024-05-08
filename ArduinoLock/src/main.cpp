#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>

// 4x4 Keypad configuration
const byte ROWS = 4; // four rows
const byte COLS = 4; // four columns
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
byte rowPins[ROWS] = {6, 7, 8, 9}; // row pins connected to R1, R2, R3, R4
byte colPins[COLS] = {2, 3, 4, 5}; // column pins connected to C1, C2, C3, C4

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// RGB LED pins
const int redPin = 10;
const int greenPin = 11;
const int bluePin = 12;

// Active Buzzer Pin
const int buzzerPin = 13;

// I2C LCD Address and Initialization
LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust the address if needed

// Servo motor configuration
Servo myServo;
const int servoPin = A0;
int servoPosition = 0;

// Counter for display
int counter = 0;
unsigned long lastCounterUpdate = 0;
const unsigned long counterUpdateInterval = 1000; // Update every second

// RGB LED color change
unsigned long lastRGBUpdate = 0;
const unsigned long rgbUpdateInterval = 500; // Change color every half second
int rgbState = 0;

// Buzzer alert
unsigned long lastBuzzerAlert = 0;
const unsigned long buzzerAlertInterval = 3000; // Alert every 3 seconds

// Servo motor rotation
unsigned long lastServoUpdate = 0;
const unsigned long servoUpdateInterval = 50; // Increment servo position every 50 ms
bool servoIncreasing = true;

void setup()
{
  Serial.begin(9600); // UART communication with ESP32
  myServo.attach(servoPin);

  // Initialize RGB LED pins
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // Initialize Buzzer Pin
  pinMode(buzzerPin, OUTPUT);

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");

  delay(1000);
  lcd.clear();
}

void updateDisplay()
{
  lcd.setCursor(0, 0);
  lcd.print("Counter: ");
  lcd.setCursor(9, 0);
  lcd.print(counter);
}

void handleRGB()
{
  switch (rgbState)
  {
  case 0:
    analogWrite(redPin, 255);
    analogWrite(greenPin, 0);
    analogWrite(bluePin, 0);
    break;
  case 1:
    analogWrite(redPin, 0);
    analogWrite(greenPin, 255);
    analogWrite(bluePin, 0);
    break;
  case 2:
    analogWrite(redPin, 0);
    analogWrite(greenPin, 0);
    analogWrite(bluePin, 255);
    break;
  case 3:
    analogWrite(redPin, 255);
    analogWrite(greenPin, 255);
    analogWrite(bluePin, 0);
    break;
  case 4:
    analogWrite(redPin, 0);
    analogWrite(greenPin, 255);
    analogWrite(bluePin, 255);
    break;
  case 5:
    analogWrite(redPin, 255);
    analogWrite(greenPin, 0);
    analogWrite(bluePin, 255);
    break;
  case 6:
    analogWrite(redPin, 255);
    analogWrite(greenPin, 255);
    analogWrite(bluePin, 255);
    break;
  case 7:
    analogWrite(redPin, 0);
    analogWrite(greenPin, 0);
    analogWrite(bluePin, 0);
    break;
  }
  rgbState = (rgbState + 1) % 8;
}

void handleBuzzerAlert()
{
  tone(buzzerPin, 1000, 200); // Buzz at 1 kHz for 200 ms
}

void handleServo()
{
  if (servoIncreasing)
  {
    servoPosition += 1;
    if (servoPosition >= 180)
    {
      servoPosition = 180;
      servoIncreasing = false;
    }
  }
  else
  {
    servoPosition -= 1;
    if (servoPosition <= 0)
    {
      servoPosition = 0;
      servoIncreasing = true;
    }
  }
  myServo.write(servoPosition);
}

void loop()
{
  char key = keypad.getKey();
  if (key)
  {
    Serial.print("Key Pressed: ");
    Serial.println(key);
    lcd.setCursor(0, 1);
    lcd.print("Key Pressed: ");
    lcd.setCursor(13, 1);
    lcd.print(key);
    delay(500);
    lcd.setCursor(0, 1);
    lcd.print("                "); // Clear the line
  }

  unsigned long currentMillis = millis();

  // Update counter on display every second
  if (currentMillis - lastCounterUpdate >= counterUpdateInterval)
  {
    lastCounterUpdate = currentMillis;
    counter++;
    updateDisplay();
  }

  // Change RGB LED color every half second
  if (currentMillis - lastRGBUpdate >= rgbUpdateInterval)
  {
    lastRGBUpdate = currentMillis;
    handleRGB();
  }

  // Buzzer alert every 3 seconds
  if (currentMillis - lastBuzzerAlert >= buzzerAlertInterval)
  {
    lastBuzzerAlert = currentMillis;
    handleBuzzerAlert();
  }

  // Update servo position every 50 ms
  if (currentMillis - lastServoUpdate >= servoUpdateInterval)
  {
    lastServoUpdate = currentMillis;
    handleServo();
  }
}
