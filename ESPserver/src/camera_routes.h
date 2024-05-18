#ifndef CAMERA_ROUTES_H
#define CAMERA_ROUTES_H

#include "Arduino.h"
#include <WiFi.h>
#include <WebServer.h>
#include "esp_camera.h"
#include "esp_http_server.h"
#include "img_converters.h"
#include "camera_pins.h"
#include "base64.h"

esp_err_t capture_handler(httpd_req_t *req);
esp_err_t live_video_handler(httpd_req_t *req);
void capture_and_store_images(char *encoded_images[], size_t encoded_lengths[]);
esp_err_t reinitialize_camera();
void start_live_video();
void stop_live_video();

#endif