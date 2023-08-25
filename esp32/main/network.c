/**
 *  Functions for connecting to a WiFi network and sending HTTP requests.
 *  Based on the WiFi station example of ESP-IDF.
 */
#include "network.h"

#define WIFI_SSID CONFIG_ESP_WIFI_SSID
#define WIFI_PASSWORD CONFIG_ESP_WIFI_PASSWORD

#define WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define WIFI_H2E_ID ""
#define WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK

static EventGroupHandle_t wifi_event_group;
static const EventBits_t WIFI_CONNECTED_BIT = BIT0;
static const EventBits_t WIFI_FAILED_BIT = BIT1;
static const int MAX_NUM_CONNECTION_ATTEMPTS = 5;

static const char* TAG = "Network";

#define SERVER_IP CONFIG_ESP_REMOTE_SERVER_IP
#define SERVER_PORT CONFIG_ESP_REMOTE_SERVER_PORT

#define RESPONSE_BUF_LEN 2048
static const int REQUEST_TASK_STACK_SIZE = 4096;
static const int REQUEST_TASK_PRIORITY = 5;
static const int RECEIVING_TIMEOUT_US = 5;
static const int HTTP_INIT_FAILURE_TIMEOUT = 1000 / portTICK_PERIOD_MS;
static const int HTTP_TRANSMISSION_FAILURE_TIMEOUT = 4000 / portTICK_PERIOD_MS;
static const int HTTP_REQUEST_TIMEOUT = 250 / portTICK_PERIOD_MS;
static const TickType_t RESPONSE_SEM_TIMEOUT = 100;

const struct addrinfo hints = {
    .ai_family = AF_INET,
    .ai_socktype = SOCK_STREAM
};

static char response_buf[RESPONSE_BUF_LEN];
static SemaphoreHandle_t http_response_sem;

/**
 *  Sends the provided HTTP request to the remote server.
 * 
 *  @param args An HTTP request string.
 */
static void http_request_task(void* args)
{
    char* request = (char*) args;

    struct addrinfo* response;
    struct in_addr* addr;

    int sock, response_len;

    while (true)
    {
        // Perform DNS lookup
        int err = getaddrinfo(SERVER_IP, SERVER_PORT, &hints, &response);

        if (err != 0 || response == NULL)
        {
            ESP_LOGE(TAG, "DNS lookup failed with error %d.", err);
            vTaskDelay(HTTP_INIT_FAILURE_TIMEOUT);
            continue;
        }
        else
        {
            addr = &((struct sockaddr_in*) response->ai_addr)->sin_addr;
        }

        // Allocate socket
        sock = socket(response->ai_family, response->ai_socktype, 0);

        if (sock < 0)
        {
            ESP_LOGE(TAG, "Socket allocation failed.");
            freeaddrinfo(response);
            vTaskDelay(HTTP_INIT_FAILURE_TIMEOUT);
            continue;
        }

        // Attempt to connect
        if (connect(sock, response->ai_addr, response->ai_addrlen) != 0)
        {
            ESP_LOGE(TAG, "Socket connection failed.");
            close(sock);
            freeaddrinfo(response);
            vTaskDelay(HTTP_TRANSMISSION_FAILURE_TIMEOUT);
            continue;
        }

        freeaddrinfo(response);

        // Send HTTP request
        if (write(sock, request, strlen(request)) < 0)
        {
            ESP_LOGE(TAG, "Failed to send request.");
            close(sock);
            vTaskDelay(HTTP_TRANSMISSION_FAILURE_TIMEOUT);
            continue;
        }

        // Configure response receiving timeout
        struct timeval receiving_timeout = {
            .tv_sec = 0,
            .tv_usec = RECEIVING_TIMEOUT_US};

        if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout, sizeof(receiving_timeout)) < 0)
        {
            ESP_LOGE(TAG, "Failed to set response receiving timeout.");
            close(sock);
            vTaskDelay(HTTP_TRANSMISSION_FAILURE_TIMEOUT);
            continue;
        }

        // Store response
        bzero(response_buf, sizeof(response_buf));
        response_len = read(sock, response_buf, sizeof(response_buf) - 1);
        xSemaphoreGive(http_response_sem);

        close(sock);
        vTaskDelay(HTTP_REQUEST_TIMEOUT);

        // Terminate task
        vTaskDelete(NULL);
    }
}

static void wifi_event_handler(void* args, esp_event_base_t event_base, int32_t event_id, void* data)
{
    static int connection_attempt_num = 0;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        // First connection attempt
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        // Attempt to reconnect to the network
        if (connection_attempt_num < MAX_NUM_CONNECTION_ATTEMPTS)
        {
            esp_wifi_connect();
            connection_attempt_num++;
        }
        else
        {
            // Indicate a connection failure if all connection attempts were used up
            xEventGroupSetBits(wifi_event_group, WIFI_FAILED_BIT);
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        // Display the IP address of the microcontroller, and indicate a successful connection
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) data;
        ESP_LOGI(TAG, "IP: " IPSTR, IP2STR(&event->ip_info.ip));
        connection_attempt_num = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void set_up_nvs()
{
    esp_err_t err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
}

void set_up_wifi()
{
    set_up_nvs();

    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize WiFi
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t init_conf = WIFI_INIT_CONFIG_DEFAULT();
    // ESP_ERROR_CHECK(esp_wifi_init(&init_conf));
    int err = esp_wifi_init(&init_conf);

    // Set up event handlers
    esp_event_handler_instance_t any_id_handler;
    esp_event_handler_instance_t got_ip_handler;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler, NULL, &any_id_handler));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler, NULL, &got_ip_handler));

    // Configure WiFi station
    wifi_config_t wifi_conf = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            .threshold.authmode = WIFI_SCAN_AUTH_MODE_THRESHOLD,
            .sae_pwe_h2e = WIFI_SAE_MODE,
            .sae_h2e_identifier = WIFI_H2E_ID,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_conf));

    // Try to connect to the WiFi network
    ESP_ERROR_CHECK(esp_wifi_start());
    EventBits_t event_bits = xEventGroupWaitBits(wifi_event_group,
                                                 WIFI_CONNECTED_BIT | WIFI_FAILED_BIT,
                                                 pdFALSE, pdFALSE, portMAX_DELAY);

    if (event_bits & WIFI_CONNECTED_BIT)
        ESP_LOGI(TAG, "Successfully connected to the WiFi network.");
    else if (event_bits & WIFI_FAILED_BIT)
        ESP_LOGE(TAG, "Failed to connect to the WiFi network.");
    else
        ESP_LOGE(TAG, "An unexpected event occurred while trying to connect to the network.");

    http_response_sem = xSemaphoreCreateBinary();
}

char* send_request(char* request)
{
    xSemaphoreTake(http_response_sem, RESPONSE_SEM_TIMEOUT);
    ESP_LOGI(TAG, "Sending HTTP request...");
    xTaskCreate(&http_request_task, "http_request_task", REQUEST_TASK_STACK_SIZE,
                (void*) request, REQUEST_TASK_PRIORITY, NULL);

    xSemaphoreTake(http_response_sem, RESPONSE_SEM_TIMEOUT);
    ESP_LOGI(TAG, "Received HTTP response.");
    xSemaphoreGive(http_response_sem);

    return &response_buf;
}
