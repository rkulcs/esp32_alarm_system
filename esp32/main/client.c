#include "client.h"

#define SERVER_IP CONFIG_ESP_REMOTE_SERVER_IP
#define SERVER_PORT CONFIG_ESP_REMOTE_SERVER_PORT

#define EMAIL CONFIG_ESP_AUTH_EMAIL
#define PASSWORD CONFIG_ESP_AUTH_PASSWORD
#define MAX_TOKEN_LENGTH 1026

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

static char token[MAX_TOKEN_LENGTH];

void update_token()
{
    printf("%s", token_http_request);
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

void set_up_client()
{
    // Set up HTTP request for getting tokens
    snprintf(token_http_request, sizeof(token_http_request), "%s%s%d\r\n\r\n%s\r\n", 
        COMMON_LOGIN_HEADERS, "Content-Length: ", strlen(LOGIN_BODY), LOGIN_BODY);
    
    update_token();
}
