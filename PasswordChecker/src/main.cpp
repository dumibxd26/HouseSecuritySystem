#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include "./Keypad/Keypad.h"

// // Define the rows and columns of the keyboard matrix
// const int numRows = 4;
// const int numCols = 4;

// // Define the pins for rows and columns of the keypad
// const int rowPins[numRows] = {22, 23, 24, 25}; // Pins for rows
// const int colPins[numCols] = {26, 27, 28, 29}; // Pins for columns

// Define the pin for the PIR sensor
const int pirPin = 53;

// Define pin numbers
const int redPin = 6;
const int greenPin = 5;
const int bluePin = 4;

// // Define the keys on the keyboard in your desired order
// char keys[numRows][numCols] = {
//     {'1', '2', '3', 'A'},
//     {'4', '5', '6', 'B'},
//     {'7', '8', '9', 'C'},
//     {'*', '0', '#', 'D'}};

int counter = 0;

const int buzzerPin = 48;

// Create a Servo object
Servo myServo;

// Define the pin for the servo control signal
const int servoPin = 52; // Use any available digital pin

// Initialize the LiquidCrystal_I2C library with the address of the display
LiquidCrystal_I2C lcd(0x27, 16, 2); // Address 0x27, 16 columns, 2 rows

// Function prototypes
// void initializeKeypad();
void initializePIR();
// char getKey();
// void printPressedKey(char key);
// bool checkMotion();
void initializeServo();
void moveServoToPosition(int position);
void initializeDisplay();
void updateDisplay();
void initializeRGBLed();
void setRGBColor(int redValue, int greenValue, int blueValue);
void turnOffRGBLed();
void initializeBuzzer();
void emitSound(int frequency, int duration);

const int numRows = 4;
const int numCols = 4;
const int rowPins[numRows] = {22, 23, 24, 25}; // Pins for rows
const int colPins[numCols] = {26, 27, 28, 29}; // Pins for columns
const char *keys[] = {
    "123A",
    "456B",
    "789C",
    "*0#D"};

// Keypad(const int numRows, const int numCols, const int *rowPins, const int *colPins, char **keys);

Keypad myKeypad(numRows, numCols, rowPins, colPins, keys);

void setup()
{
  // Initialize serial communication
  Serial.begin(9600);

  // Initialize the keypad
  // initializeKeypad();
  myKeypad.initialize();

  // Initialize the PIR sensor
  initializePIR();

  // Initialize the servo
  initializeServo();

  // Initialize the display
  initializeDisplay();

  // Initialize the RGB LED
  initializeRGBLed();

  // Initialize buzzer
  initializeBuzzer();
}

void loop()
{
  // Check for motion
  // if (checkMotion())
  // {
  //   counter++;
  //   Serial.println("Motion detected! Count: " + String(counter));
  //   delay(1000); // Wait for a second to avoid multiple detections
  // }

  // Check for keypad input
  // char key = getKey();
  // if (key != '\0')
  // {
  //   printPressedKey(key);
  //   delay(300); // Debounce delay
  // }

  // // Update the display
  // updateDisplay();

  //   setRGBColor(255, 0, 0); // Red
  //   delay(1000);

  //   setRGBColor(0, 255, 0); // Green
  //   delay(1000);

  //   setRGBColor(0, 0, 255); // Blue
  //   delay(1000);

  //   setRGBColor(255, 255, 255); // White
  //   delay(1000);

  //   turnOffRGBLed(); // Turn off
  //   delay(1000);

  // Make the buzzer emit sound
  // emitSound(1000, 1000); // 1000 Hz frequency for 1 second
  // delay(1000);           // Wait for 1 second

  // digitalWrite(buzzerPin, HIGH);
  // delay(2000); // Keep it on for 1 second

  // // Turn the buzzer off
  // digitalWrite(buzzerPin, LOW);
  // delay(500);

  // // print the keys from the keypad to the serial monitor
  // char key = myKeypad.getKey();
  // if (key != myKeypad.getLastKeyPressed())
  // { // Check if a key is pressed and no key is currently held down
  //   myKeypad.setLastKeyPressed(key);
  //   myKeypad.printPressedKey(key);
  // }
}

// Function to initialize the keypad pins
// void initializeKeypad()
// {
//   // Set row pins as inputs and enable pull-up resistors
//   for (int i = 0; i < numRows; i++)
//   {
//     pinMode(rowPins[i], INPUT_PULLUP);
//   }

//   // Set column pins as outputs
//   for (int i = 0; i < numCols; i++)
//   {
//     pinMode(colPins[i], OUTPUT);
//     digitalWrite(colPins[i], HIGH); // Set columns to high to disable
//   }
// }

// Function to initialize the PIR sensor
void initializePIR()
{
  pinMode(pirPin, INPUT);
}

// // Function to scan the keypad and return the pressed key
// char getKey()
// {
//   for (int col = 0; col < numCols; col++)
//   {
//     // Enable the current column
//     digitalWrite(colPins[col], LOW);

//     // Check each row in this column
//     for (int row = 0; row < numRows; row++)
//     {
//       // If a key is pressed (LOW), return its value
//       if (digitalRead(rowPins[row]) == LOW)
//       {
//         // Disable the current column
//         digitalWrite(colPins[col], HIGH);
//         return keys[row][col];
//       }
//     }

//     // Disable the current column
//     digitalWrite(colPins[col], HIGH);
//   }
//   return '\0'; // Return null if no key is pressed
// }

// // Function to print the pressed key
// void printPressedKey(char key)
// {
//   Serial.println("Key: " + String(key));
//   lcd.setCursor(0, 1); // Set cursor to the first column of the second row
//   lcd.print("Key: ");
//   lcd.print(key); // Print the pressed key
// }

// Function to check for motion
// bool checkMotion()
// {
//   return digitalRead(pirPin) == HIGH;
// }

// Function to initialize the servo
void initializeServo()
{
  myServo.attach(servoPin); // Attach the servo to the pin
}

// Function to move the servo to a specific position
void moveServoToPosition(int position)
{
  myServo.write(position);
}

// Function to initialize the display
void initializeDisplay()
{
  lcd.init();                  // Initialize the display
  delay(1000);                 // Wait for the display to power up
  lcd.backlight();             // Turn on the backlight
  lcd.setCursor(0, 0);         // Set cursor to the first column of the first row
  lcd.print("Motion Sensor:"); // Print initial message`
}

// Function to update the display
void updateDisplay()
{
  lcd.setCursor(0, 1);                    // Set cursor to the first column of the second row
  lcd.print("Count: " + String(counter)); // Print count
}

// Function to initialize RGB LED pins
void initializeRGBLed()
{
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
}

void setRGBColor(int redValue, int greenValue, int blueValue)
{
  // Map values from 0-255 to 0-1023 (Arduino Mega PWM range)
  int redPWM = map(redValue, 0, 255, 0, 1023);
  int greenPWM = map(greenValue, 0, 255, 0, 1023);
  int bluePWM = map(blueValue, 0, 255, 0, 1023);

  // Set PWM values for each color pin
  analogWrite(redPin, redPWM);
  analogWrite(greenPin, greenPWM);
  analogWrite(bluePin, bluePWM);
}

void turnOffRGBLed()
{
  // Set all color pins to LOW
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, LOW);
}

// Function to initialize buzzer pin
void initializeBuzzer()
{
  pinMode(buzzerPin, OUTPUT);
  // noTone(buzzerPin); // Stop any initial sound
}

// Function to emit sound
void emitSound(int frequency, int duration)
{
  tone(buzzerPin, frequency); // Generate tone of specified frequency
  delay(duration);            // Sound for specified duration
  // noTone(buzzerPin);          // Stop sound
}