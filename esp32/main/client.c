#include "client.h"

#define SERVER_IP CONFIG_ESP_REMOTE_SERVER_IP
#define SERVER_PORT CONFIG_ESP_REMOTE_SERVER_PORT

#define EMAIL CONFIG_ESP_AUTH_EMAIL
#define PASSWORD CONFIG_ESP_AUTH_PASSWORD
#define MAX_TOKEN_LENGTH 1026

static const char* TAG = "Client";

static const char* COMMON_LOGIN_HEADERS = "POST /login HTTP/1.1\r\n"
                    "Host: " SERVER_IP ":" SERVER_PORT "\r\n"
                    "User-Agent: esp-idf/1.0 esp32\r\n"
                    "Content-Type: application/json\r\n";
static const char* LOGIN_BODY = "{\"email\": \"" EMAIL "\", \"password\": \"" PASSWORD "\"}";

static const char* COMMON_ALARM_HEADERS = "POST /post-alarm HTTP/1.1\r\n"
                    "Host: " SERVER_IP ":" SERVER_PORT "\r\n"
                    "User-Agent: esp-idf/1.0 esp32\r\n"
                    "Content-Type: application/json\r\n";

static char token_http_request[256];
static char alarm_http_request[MAX_TOKEN_LENGTH + 256];

static char token[MAX_TOKEN_LENGTH];
static char auth_header[MAX_TOKEN_LENGTH + 15];

static const int ALARM_TIMEOUT = 250 / portTICK_PERIOD_MS;

static const char* ALARM_REQUEST_SUCCESS_CODE = "202";

void update_token()
{
    char* response = send_request(token_http_request);

    // Parse each line of the response, and store the last line as the token
    char* tmp = strtok(response, "\n");
    char* token_substr = NULL;

    while (tmp != NULL)
    {
        token_substr = tmp;
        tmp = strtok(NULL, "\n");
    }

    strlcpy(token, token_substr, sizeof(token));
}

void update_auth_header()
{
    bzero(auth_header, sizeof(auth_header));
    snprintf(auth_header, sizeof(auth_header), "%s%s", "Authorization: ", token);
}

void set_up_client()
{
    // Set up HTTP request for getting tokens
    snprintf(token_http_request, sizeof(token_http_request), "%s%s%d\r\n\r\n%s\r\n", 
        COMMON_LOGIN_HEADERS, "Content-Length: ", strlen(LOGIN_BODY), LOGIN_BODY);
    
    update_token();
    update_auth_header();
}

void send_alarm_message()
{
    static char* remaining_headers = "Content-Length: 32\r\n"
                                    "\r\n"
                                    "{\"subject\": \"Intruder detected\"}";
    bzero(alarm_http_request, sizeof(alarm_http_request));
    snprintf(alarm_http_request, sizeof(alarm_http_request), "%s%s\r\n%s", 
            COMMON_ALARM_HEADERS, auth_header, remaining_headers);

    while (true)
    {
        char* response = send_request(alarm_http_request);

        // Check the status code in the first line of the response
        char* first_line = strtok(response, "\n");

        if (strtok(first_line, ALARM_REQUEST_SUCCESS_CODE) != NULL)
        {
            ESP_LOGI(TAG, "Successfully sent the alarm message to the server.");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to send the alarm message to the server.");
            continue;
        }
        
        vTaskDelay(ALARM_TIMEOUT);
        vTaskDelete(NULL);
    }
}
