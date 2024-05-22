#include "arduino_routes.h"

esp_err_t activate_alarm_handler(httpd_req_t *req)
{
    Serial.println("Aon");
    return httpd_resp_send(req, "Alarm activated", HTTPD_RESP_USE_STRLEN);
}

esp_err_t deactivate_alarm_handler(httpd_req_t *req)
{
    Serial.println("Aoff");
    return httpd_resp_send(req, "Alarm deactivated", HTTPD_RESP_USE_STRLEN);
}

#include "cJSON.h"

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
    if (new_password == NULL)
    {
        Serial.println("new_password key not found");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "new_password key missing");
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    // Treat the value as a string and check its length
    const char *password = cJSON_IsString(new_password) ? new_password->valuestring : cJSON_PrintUnformatted(new_password);
    if (strlen(password) <= 3)
    {
        Serial.println("new_password is too short");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "new_password must be longer than 3 characters");
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    cJSON_Delete(root);

    const char *resp_str = "{\"message\":\"Password changed successfully\"}";
    httpd_resp_set_type(req, "application/json");

    // send the new password to the Arduino
    Serial.println("newPassword " + String(password));

    return httpd_resp_send(req, resp_str, strlen(resp_str));
}

esp_err_t allow_access_handler(httpd_req_t *req)
{
    Serial.println("allowAccess");
    return httpd_resp_send(req, "Access allowed", HTTPD_RESP_USE_STRLEN);
}