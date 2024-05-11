#include "camera_routes.h"

esp_err_t reinitialize_camera()
{
    extern camera_config_t camera_config;
    esp_err_t err = esp_camera_deinit();
    if (err != ESP_OK)
    {
        Serial.printf("Camera deinit failed with error 0x%x\n", err);
        return err;
    }

    err = esp_camera_init(&camera_config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x\n", err);
    }
    return err;
}

// Function to start live video streaming
void start_live_video()
{
    extern camera_config_t camera_config;
    camera_config.frame_size = FRAMESIZE_VGA; // Change to a smaller frame size for video
    esp_err_t err = reinitialize_camera();
    if (err != ESP_OK)
    {
        Serial.println("Failed to reinitialize camera for live video.");
    }
}

// Function to stop live video streaming
void stop_live_video()
{
    extern camera_config_t camera_config;
    camera_config.frame_size = FRAMESIZE_SVGA; // Change back to original frame size
    esp_err_t err = reinitialize_camera();
    if (err != ESP_OK)
    {
        Serial.println("Failed to reinitialize camera after live video.");
    }
}

// Handler to capture and send an image directly in chunks
esp_err_t capture_handler(httpd_req_t *req)
{
    Serial.println("Accessed capture handler");

    // Capture a single image
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb)
    {
        Serial.println("Camera capture failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    // Set HTTP headers for JPEG image
    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");

    // Stream the image directly in chunks
    esp_err_t res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
    if (res != ESP_OK)
    {
        Serial.println("Failed to send image");
    }

    // Finalize the HTTP response
    httpd_resp_send_chunk(req, NULL, 0);
    esp_camera_fb_return(fb); // return the frame buffer back to the driver for reuse
    return res;
}

// Handler to show all three images captured
// esp_err_t capture_handler(httpd_req_t *req)
// {
//     Serial.println("Accessed capture handler");
//     // Buffers to store encoded images
//     char *encoded_images[3] = {NULL, NULL, NULL};
//     size_t encoded_lengths[3] = {0, 0, 0};

//     // Capture three images and store them
//     capture_and_store_images(encoded_images, encoded_lengths);

//     String html_response = "<html>"
//                            "<head><title>Captured Images</title></head>"
//                            "<body>"
//                            "<h1>Captured Images from ESP32-CAM</h1>";

//     for (int i = 0; i < 3; i++)
//     {
//         if (encoded_images[i] != NULL)
//         {
//             html_response += "<img src=\"data:image/jpeg;base64,";
//             html_response += encoded_images[i];
//             html_response += "\"/><br/><br/>";
//             free(encoded_images[i]);
//         }
//         else
//         {
//             html_response += "<p>Image ";
//             html_response += String(i + 1);
//             html_response += " not available</p><br/>";
//         }
//     }

//     html_response += "<a href=\"/\"><button>Back to Home</button></a>"
//                      "</body>"
//                      "</html>";

//     esp_err_t res = httpd_resp_set_type(req, "text/html");
//     if (res == ESP_OK)
//     {
//         res = httpd_resp_send(req, html_response.c_str(), html_response.length());
//     }
//     return res;
// }

// // Capture and store three images
// void capture_and_store_images(char *encoded_images[], size_t encoded_lengths[])
// {
//     Serial.println("Capturing and storing images");
//     for (int i = 0; i < 3; i++)
//     {
//         camera_fb_t *fb = NULL;
//         // Turn on the flash LED
//         digitalWrite(FLASH_GPIO_PIN, HIGH);
//         delay(100); // Short delay to allow the flash to illuminate
//         fb = esp_camera_fb_get();
//         // Turn off the flash LED
//         digitalWrite(FLASH_GPIO_PIN, LOW);

//         if (!fb)
//         {
//             Serial.printf("Capture failed for image %d\n", i + 1);
//             if (encoded_images[i] != NULL)
//             {
//                 free(encoded_images[i]);
//                 encoded_images[i] = NULL;
//             }
//             encoded_lengths[i] = 0;
//             delay(1000); // Wait before retrying
//             continue;
//         }

//         // Base64 encode the image
//         if (encoded_images[i] != NULL)
//         {
//             free(encoded_images[i]);
//         }
//         encoded_images[i] = (char *)malloc((4 * ((fb->len + 2) / 3)) + 1);
//         if (encoded_images[i] == NULL)
//         {
//             Serial.println("Memory allocation failed for Base64 encoding");
//             esp_camera_fb_return(fb);
//             encoded_lengths[i] = 0;
//             delay(1000); // Wait before retrying
//             continue;
//         }

//         if (base64_encode((unsigned char *)encoded_images[i], (unsigned char *)fb->buf, fb->len, &encoded_lengths[i]) != ESP_OK)
//         {
//             Serial.printf("Base64 encoding failed for image %d\n", i + 1);
//             esp_camera_fb_return(fb);
//             free(encoded_images[i]);
//             encoded_images[i] = NULL;
//             encoded_lengths[i] = 0;
//             delay(1000); // Wait before retrying
//             continue;
//         }

//         encoded_images[i][encoded_lengths[i]] = '\0';
//         esp_camera_fb_return(fb);

//         Serial.printf("Captured and stored image %d\n", i + 1);
//         delay(1000); // Wait for 1 second between captures
//     }
// }

// Handler to stream live video
esp_err_t live_video_handler(httpd_req_t *req)
{

    Serial.println("Accessed live video handler");
    start_live_video();
    char *boundary = "frame";
    char part_buf[64];
    unsigned long previousMillis = 0;
    const unsigned long interval = 100; // Increase frame interval to 100 ms for better stability

    // Set HTTP response headers
    httpd_resp_set_type(req, "multipart/x-mixed-replace;boundary=frame");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    while (true)
    {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis < interval)
        {
            delay(interval - (currentMillis - previousMillis)); // Efficient wait
            continue;
        }
        previousMillis = currentMillis; // Update previousMillis

        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb)
        {
            Serial.println("Camera capture failed");
            continue;
        }

        size_t len = snprintf(part_buf, 64, "\r\n--%s\r\nContent-Type: image/jpeg\r\nContent-Length: %zu\r\n\r\n", boundary, fb->len);
        if (httpd_resp_send_chunk(req, part_buf, len) != ESP_OK || httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len) != ESP_OK)
        {
            esp_camera_fb_return(fb);
            break;
        }
        esp_camera_fb_return(fb);
    }

    // End the multipart response
    httpd_resp_send_chunk(req, "\r\n--frame--\r\n", strlen("\r\n--frame--\r\n"));

    stop_live_video();
    return ESP_OK;
}