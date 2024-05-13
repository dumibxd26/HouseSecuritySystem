#include "FS.h"
#include "SPIFFS.h"
#include <unordered_map>
#include <string>

std::unordered_map<std::string, std::string> readCredentials(const std::string &filename)
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("An error occurred while mounting SPIFFS");
        return {};
    }

    File file = SPIFFS.open(filename.c_str(), "r");
    std::unordered_map<std::string, std::string> credentials;

    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return {};
    }

    while (file.available())
    {
        String line = file.readStringUntil('\n');
        int pos = line.indexOf('=');
        if (pos != -1)
        {
            String key = line.substring(0, pos);
            String value = line.substring(pos + 1);
            credentials[key.c_str()] = value.c_str();
        }
    }
    file.close();

    return credentials;
}
