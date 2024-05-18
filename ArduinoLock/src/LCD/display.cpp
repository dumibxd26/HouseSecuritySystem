#include "display.h"

Display::Display() : lcd(0x27, 16, 2)
{
}

void Display::initialize()
{
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Enter Password:");
}

void Display::printMessage(bool type, const char *message, const char *second_line_message = NULL)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(message);

    switch (type)
    {
    case ONE_LINE_MESSAGE:
        break;
    case TWO_LINE_MESSAGE:
        lcd.setCursor(0, 1);
        lcd.print(second_line_message);
        break;
    }
}

void Display::noBacklight()
{
    lcd.noBacklight();
}

void Display::backlight()
{
    lcd.backlight();
}

void Display::clear()
{
    lcd.clear();
}