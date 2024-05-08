#include "esp_camera.h"
#include "WiFi.h"
#include "esp_http_server.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "camera_pins.h"
#include "FS.h"
#include "SPIFFS.h"

httpd_handle_t camera_httpd = NULL;

// Function prototypes
static esp_err_t index_handler(httpd_req_t *req);
void startCameraServer();
static esp_err_t capture_handler(httpd_req_t *req);

// WiFi credentials
const char *ssid = "NONO";
const char *password = "NICI_ATAT";

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting up...");

  // Initialize filesystem
  if (!SPIFFS.begin(true))
  {
    Serial.println("An error occurred while mounting SPIFFS");
  }
  else
  {
    Serial.println("SPIFFS mounted successfully");
  }

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.println("Attempting to connect to WiFi network...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

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
      .frame_size = FRAMESIZE_UXGA,
      .jpeg_quality = 12,
      .fb_count = 2};

  // Initialize the camera
  esp_err_t err = esp_camera_init(&camera_config);
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  }

  // Start the web server
  startCameraServer();
  Serial.println("Camera Ready! Use 'http://" + String(WiFi.localIP()) + "/' to connect");
}

void loop()
{
  // Print the IP address of the ESP32-CAM
  Serial.print("IP Address: " + WiFi.localIP().toString());
  delay(1000);
}

void startCameraServer()
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

  // Start the web server and register URI handlers
  if (httpd_start(&camera_httpd, &config) == ESP_OK)
  {
    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &capture_uri);
  }
}

// Handler for HTTP GET requests to the root URI
static esp_err_t index_handler(httpd_req_t *req)
{
  const char *resp_str = "<html>"
                         "<head><title>ESP32-CAM</title></head>"
                         "<body>"
                         "<h1>Hello from ESP32-CAM!</h1>"
                         "<p>Click the button below to capture an image.</p>"
                         "<a href=\"/capture\"><button>Capture Image</button></a>"
                         "</body>"
                         "</html>";
  httpd_resp_set_type(req, "text/html");                        // Set content type as HTML
  return httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN); // Send the HTML content
}

// Handler to capture and send an image
static esp_err_t capture_handler(httpd_req_t *req)
{
  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;
  size_t fb_len = 0;

  fb = esp_camera_fb_get();
  if (!fb)
  {
    Serial.println("Camera capture failed");
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  res = httpd_resp_set_type(req, "image/jpeg");
  if (res == ESP_OK)
  {
    res = httpd_resp_send(req, (const char *)fb->buf, fb->len); // send the actual image
  }
  if (fb)
  {
    esp_camera_fb_return(fb); // return the frame buffer back to the driver for reuse
  }
  return res;
}
