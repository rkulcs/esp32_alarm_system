/**
 * Functions for connecting to a WiFi network and sending HTTP requests.
 * Based on the WiFi station example of ESP-IDF.
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

    // TODO: Remove if not needed
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize WiFi
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t init_conf = WIFI_INIT_CONFIG_DEFAULT();
    // ESP_ERROR_CHECK(esp_wifi_init(&init_conf));
    int err = esp_wifi_init(&init_conf);
    printf("%d\n", err);

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
}
