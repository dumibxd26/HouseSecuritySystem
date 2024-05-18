#include "arduino_routes.h"

esp_err_t activate_alarm_handler(httpd_req_t *req)
{
    Serial.println("Aon"); // Send 'D1' to Arduino for activation
    return httpd_resp_send(req, "Alarm activated", HTTPD_RESP_USE_STRLEN);
}

esp_err_t deactivate_alarm_handler(httpd_req_t *req)
{
    Serial.println("Aoff"); // Send 'D0' to Arduino for deactivation
    return httpd_resp_send(req, "Alarm deactivated", HTTPD_RESP_USE_STRLEN);
}