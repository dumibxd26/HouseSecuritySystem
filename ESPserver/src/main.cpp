#include "esp_camera.h"
#include "WiFi.h"
#include "esp_http_server.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "camera_routes.h"
#include "arduino_routes.h"
#include "utils.h"
#include "WiFiCredentials.h"
#include "esp_wifi.h"
#include "esp_bt.h"

// WiFi credentials
const char *ssid = SSID_PHONE;
const char *password = PASSWORD_PHONE;

// GPIO pin for flash LED
#define FLASH_GPIO_PIN 4
#define WAKEUP_PIN GPIO_NUM_12 // Use GPIO 12 for wakeup

// Handle for the HTTP server
httpd_handle_t camera_httpd = NULL;

// Function prototypes
void startServer();
static esp_err_t index_handler(httpd_req_t *req);
void startWiFi();
void stopWiFi();

// Camera configuration
camera_config_t camera_config = {
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_sscb_sda = SIOD_GPIO_NUM,
    .pin_sscb_scl = SIOC_GPIO_NUM,
    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_SVGA,
    .jpeg_quality = 10,
    .fb_count = 2};

unsigned long wifiStartMillis = 0;
bool wifiConnected = false;

void setup()
{
  Serial.begin(9600);
  // Turn off Bluetooth for optimization
  btStop();

  // Initialize flash GPIO pin
  pinMode(FLASH_GPIO_PIN, OUTPUT);
  digitalWrite(FLASH_GPIO_PIN, LOW);

  // Initialize the camera
  esp_err_t err = esp_camera_init(&camera_config);
  if (err != ESP_OK)
  {
    return;
  }

  // Configure GPIO12 as input
  pinMode(WAKEUP_PIN, INPUT);

  // Start WiFi and web server
  // startWiFi();
  // startServer();
}

void loop()
{
  // If GPIO12 is LOW, reconnect to WiFi and start the timer
  if (digitalRead(WAKEUP_PIN) == HIGH)
  {
    // start the led
    digitalWrite(FLASH_GPIO_PIN, HIGH);

    if (!wifiConnected)
    {
      startWiFi();
      startServer();
      wifiConnected = true;
      wifiStartMillis = millis();
    }
  }

  // If 100 seconds have passed since last activity, disconnect from WiFi
  if (wifiConnected && (millis() - wifiStartMillis > 100000))
  {
    stopWiFi();
    wifiConnected = false;
  }
}

void startWiFi()
{
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
}

void stopWiFi()
{
  // Disconnect from WiFi
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void startServer()
{
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;

  httpd_uri_t index_uri = {
      .uri = "/",
      .method = HTTP_GET,
      .handler = index_handler,
      .user_ctx = NULL};

  httpd_uri_t capture_uri = {
      .uri = "/capture",
      .method = HTTP_GET,
      .handler = capture_handler,
      .user_ctx = NULL};

  httpd_uri_t live_video_uri = {
      .uri = "/live_video",
      .method = HTTP_GET,
      .handler = live_video_handler,
      .user_ctx = NULL};

  httpd_uri_t activate_alarm_uri = {
      .uri = "/activate_alarm",
      .method = HTTP_GET,
      .handler = activate_alarm_handler,
      .user_ctx = NULL};

  httpd_uri_t deactivate_alarm_uri = {
      .uri = "/deactivate_alarm",
      .method = HTTP_GET,
      .handler = deactivate_alarm_handler,
      .user_ctx = NULL};

  httpd_uri_t change_password_uri = {
      .uri = "/change_password",
      .method = HTTP_POST,
      .handler = change_password_handler,
      .user_ctx = NULL};

  httpd_uri_t allow_access_uri = {
      .uri = "/allow_access",
      .method = HTTP_GET,
      .handler = allow_access_handler,
      .user_ctx = NULL};

  // Start the web server and register URI handlers
  if (httpd_start(&camera_httpd, &config) == ESP_OK)
  {
    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &capture_uri);
    httpd_register_uri_handler(camera_httpd, &live_video_uri);
    httpd_register_uri_handler(camera_httpd, &activate_alarm_uri);
    httpd_register_uri_handler(camera_httpd, &deactivate_alarm_uri);
    httpd_register_uri_handler(camera_httpd, &change_password_uri);
    httpd_register_uri_handler(camera_httpd, &allow_access_uri);
  }
}

// Handler for HTTP GET requests to the root URI
static esp_err_t index_handler(httpd_req_t *req)
{
  const char *resp_str = "<html>"
                         "<head><title>ESP32-CAM</title></head>"
                         "<body>"
                         "<h1>ESP32-CAM Capture</h1>"
                         "<p>Click the button below to capture images or stream live video.</p>"
                         "<a href=\"/capture\"><button>Capture and View Images</button></a><br/><br/>"
                         "<a href=\"/live_video\"><button>Stream Live Video</button></a><br/><br/>"
                         "<a href=\"/activate_alarm\"><button>Activate Alarm</button></a><br/><br/>"
                         "<a href=\"/deactivate_alarm\"><button>Deactivate Alarm</button></a><br/><br/>"
                         "</body>"
                         "</html>";
  httpd_resp_set_type(req, "text/html");                        // Set content type as HTML
  return httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN); // Send the HTML content
}
