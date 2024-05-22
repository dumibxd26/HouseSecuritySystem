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

esp_err_t change_password_handler(httpd_req_t *req)
{
    char content[100];
    int ret = httpd_req_recv(req, content, std::min(req->content_len, sizeof(content) - 1));
    if (ret <= 0)
    { // If no content received, return error
        if (ret == HTTPD_SOCK_ERR_TIMEOUT)
        {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    content[ret] = '\0'; // Null-terminate the received content

    // Parse JSON
    cJSON *root = cJSON_Parse(content);
    if (root == NULL)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }

    cJSON *new_password = cJSON_GetObjectItem(root, "new_password");
    if (new_password == NULL || !cJSON_IsString(new_password))
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "new_password key missing or not a string");
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    const char *password = new_password->valuestring;
    Serial.printf("New password: %s\n", password); // Handle the new password here

    cJSON_Delete(root);

    const char *resp_str = "{\"message\":\"Password changed successfully\"}";
    httpd_resp_set_type(req, "application/json");

    // send the new password to the Arduino
    Serial.println("newPassword " + String(password));

    return httpd_resp_send(req, resp_str, strlen(resp_str));
}
