#include <LiquidCrystal_I2C.h>
#define ONE_LINE_MESSAGE 0
#define TWO_LINE_MESSAGE 1

class Display
{
private:
    LiquidCrystal_I2C lcd;

public:
    Display();
    void initialize();
    void printMessage(bool type, const char *message, const char *second_line_message = NULL);
    void clear();
};