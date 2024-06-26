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

// load .env file
// std::unordered_map<std::string, std::string> credentials = readCredentials("../.env");

// WiFi credentials
const char *ssid = SSID_PHONE;
const char *password = PASSWORD_PHONE;
bool is_arduino_active = false;
unsigned long time_arduino_start_signal = 0;

unsigned int previousMillis = 0;

// Static IP configuration
// IPAddress local_IP(192, 168, 1, 50); // IP address
// IPAddress gateway(192, 168, 1, 1);   // gateway address
// IPAddress subnet(255, 255, 255, 0);  // Subnet mask
// IPAddress primaryDNS(8, 8, 8, 8);    // Primary DNS server
// IPAddress secondaryDNS(8, 8, 4, 4);  // Secondary DNS server

httpd_handle_t camera_httpd = NULL;

// Function prototypes
void startServer();
static esp_err_t index_handler(httpd_req_t *req);

// GPIO pin for flash LED

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
    .frame_size = FRAMESIZE_SVGA, // Set to VGA (640x480) for smaller images
    .jpeg_quality = 10,           // Adjusted JPEG quality
    .fb_count = 2};

void setup()
{
  Serial.begin(9600);
  // turn the bluetooth off optimisation
  btStop();

  // Connect to WiFi with a static IP
  // if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
  // {
  //   Serial.println("STA Failed to configure");
  // }

  WiFi.begin(ssid, password);
  // serial.println("Attempting to connect to WiFi network...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    // serial.println(ssid);
    // serial.println(password);
  }

  // Initialize flash GPIO pin
  pinMode(FLASH_GPIO_PIN, OUTPUT);
  digitalWrite(FLASH_GPIO_PIN, LOW);

  // Initialize the camera
  esp_err_t err = esp_camera_init(&camera_config);
  if (err != ESP_OK)
  {
    // Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  }

  // Start the web server
  startServer();
}

void loop()
{
  // unsigned long currentMillis = millis();

  // if (currentMillis - previousMillis >= 5000)
  // {
  //   previousMillis = currentMillis;
  //   Serial.println("Routes:");
  //   Serial.println("http://" + String(WiFi.localIP()) + "/");
  //   Serial.println("http://" + String(WiFi.localIP()) + "/capture");
  //   Serial.println("http://" + String(WiFi.localIP()) + "/live_video");
  // }

  // wait for message "start" from arduino
  if (Serial.available() > 0)
  {
    String message = Serial.readString();
    if (message.startsWith("start"))
    {
      is_arduino_active = true;
      time_arduino_start_signal = millis();
    }
    else if (message.startsWith("stop"))
    {
      is_arduino_active = false;
      time_arduino_start_signal = 0;
    }
  }
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

  httpd_uri_t check_movement_uri = {
      .uri = "/check_movement",
      .method = HTTP_GET,
      .handler = check_movement_handler,
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
    httpd_register_uri_handler(camera_httpd, &check_movement_uri);
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