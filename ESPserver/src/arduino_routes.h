#ifndef ARDUINO_ROUTES_H
#define ARDUINO_ROUTES_H

#include "Arduino.h"
#include <WiFi.h>
#include <WebServer.h>
#include "esp_camera.h"
#include "esp_http_server.h"
#include "camera_pins.h"
#include "cJSON.h"
#include <algorithm>

#define RX_PIN 3
#define TX_PIN 1

esp_err_t activate_alarm_handler(httpd_req_t *req);
esp_err_t deactivate_alarm_handler(httpd_req_t *req);
esp_err_t change_password_handler(httpd_req_t *req);
esp_err_t allow_access_handler(httpd_req_t *req);

#endif