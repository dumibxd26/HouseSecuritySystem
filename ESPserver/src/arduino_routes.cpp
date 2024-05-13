#include "arduino_routes.h"

esp_err_t activate_alarm_handler(httpd_req_t *req)
{
    digitalWrite(FLASH_GPIO_PIN, HIGH); // Assuming the flash GPIO is used for the alarm
    Serial2.write("A1");                // Send 'A1' to Arduino for activation
    return httpd_resp_send(req, "Alarm activated", HTTPD_RESP_USE_STRLEN);
}

esp_err_t deactivate_alarm_handler(httpd_req_t *req)
{
    digitalWrite(FLASH_GPIO_PIN, LOW); // Turn off the alarm
    Serial2.write("D0");               // Send 'D0' to Arduino for deactivation
    return httpd_resp_send(req, "Alarm deactivated", HTTPD_RESP_USE_STRLEN);
}