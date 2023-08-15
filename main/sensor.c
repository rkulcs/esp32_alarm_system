#include "sensor.h"

static const char* TAG = "Sensor";

static const int PIN_TRIG = 13;
static const int PIN_ECHO = 14;

static const int TIMER_CONF_GROUP_ID = 0;
static const int INPUT_SIGNAL_PRESCALE = 1;
static const uint64_t TRIG_PIN_BITMASK = 1ULL << PIN_TRIG;

static const int TRIG_PERIOD_US = 10;
static const int SENSOR_TIMEOUT_MS = 250 / portTICK_PERIOD_MS;

// The approximate number of microseconds needed by sound to travel 1 cm at room temperature.
static const double SOUND_US_PER_CM = 58.0;

static const int SENSOR_SETUP_NUM_SAMPLES = 16;
static const int MAX_OUT_OF_RANGE_DISTANCES = 2;
static const int SENSING_NUM_SAMPLES = 4;
static const double SAFE_DISTANCE_TOLERANCE_CM = 2.0;

struct
{
    mcpwm_cap_timer_handle_t timer;
    mcpwm_capture_timer_config_t timer_conf;
    mcpwm_cap_channel_handle_t channel;
    mcpwm_capture_channel_config_t channel_conf;
    TaskHandle_t task_handle;
    mcpwm_capture_event_callbacks_t callbacks;
    gpio_config_t gpio_conf;

    double min_safe_distance;
    double max_safe_distance;
} sensor;

static bool sensor_callback(mcpwm_cap_channel_handle_t channel, 
                            const mcpwm_capture_event_data_t* event_data, void* data)
{
    static uint32_t start = 0;
    static uint32_t end = 0;

    TaskHandle_t task_to_notify = (TaskHandle_t) data;
    BaseType_t successful_notification = pdFALSE;

    if (event_data->cap_edge == MCPWM_CAP_EDGE_POS)
    {
        // Get the timestamp at the positive edge of the pulse
        start = event_data->cap_value;
        end = start;
    }
    else
    {
        // Get the timestamp at the negative edge of the pulse
        end = event_data->cap_value;

        // Determine the number of ticks, then notify the distance calculation task
        uint32_t ticks = end - start;
        xTaskNotifyFromISR(task_to_notify, ticks, eSetValueWithOverwrite, &successful_notification);
    }

    return (successful_notification == pdTRUE);
}

void set_up_sensor()
{
    // Set up capture timer
    sensor.timer = NULL;
    sensor.timer_conf.clk_src = MCPWM_CAPTURE_CLK_SRC_DEFAULT;
    sensor.timer_conf.group_id = TIMER_CONF_GROUP_ID;
    ESP_ERROR_CHECK(mcpwm_new_capture_timer(&(sensor.timer_conf), &(sensor.timer)));

    // Set up capture channel
    sensor.channel = NULL;
    sensor.channel_conf.gpio_num = PIN_ECHO;
    sensor.channel_conf.prescale = INPUT_SIGNAL_PRESCALE;
    sensor.channel_conf.flags.neg_edge = true;
    sensor.channel_conf.flags.pos_edge = true;
    sensor.channel_conf.flags.pull_up = true;
    ESP_ERROR_CHECK(
        mcpwm_new_capture_channel(sensor.timer, &(sensor.channel_conf), &(sensor.channel))
    );

    // Register capture callback function
    sensor.task_handle = xTaskGetCurrentTaskHandle();
    sensor.callbacks.on_cap = sensor_callback;
    ESP_ERROR_CHECK(
        mcpwm_capture_channel_register_event_callbacks(sensor.channel, &(sensor.callbacks), 
            sensor.task_handle)
    );
    ESP_ERROR_CHECK(mcpwm_capture_channel_enable(sensor.channel));

    // Configure Trig pin
    sensor.gpio_conf.mode = GPIO_MODE_OUTPUT;
    sensor.gpio_conf.pin_bit_mask = TRIG_PIN_BITMASK;
    ESP_ERROR_CHECK(gpio_config(&(sensor.gpio_conf)));
    gpio_set_level(PIN_TRIG, 0);

    // Set up capture timer
    ESP_ERROR_CHECK(mcpwm_capture_timer_enable(sensor.timer));
    ESP_ERROR_CHECK(mcpwm_capture_timer_start(sensor.timer));
}

void sensor_set_safe_distances()
{
    sensor.min_safe_distance = __INT_MAX__;
    sensor.max_safe_distance = 0;

    int time_left_ms = SENSOR_SETUP_NUM_SAMPLES * SENSOR_TIMEOUT_MS * portTICK_PERIOD_MS;

    for (int i = 0; i < SENSOR_SETUP_NUM_SAMPLES; i++)
    {
        char time_left_str[5];
        sprintf(time_left_str, "%d", time_left_ms);
        lcd_write_two_lines("Setup time left:", time_left_str);

        double distance_cm = sensor_get_distance_in_cm();

        if (distance_cm < sensor.min_safe_distance)
            sensor.min_safe_distance = distance_cm - SAFE_DISTANCE_TOLERANCE_CM;
        
        if (distance_cm > sensor.max_safe_distance)
            sensor.max_safe_distance = distance_cm + SAFE_DISTANCE_TOLERANCE_CM;

        vTaskDelay(SENSOR_TIMEOUT_MS);
        time_left_ms -= SENSOR_TIMEOUT_MS * portTICK_PERIOD_MS;
    }

    ESP_LOGI(TAG, "Minimum safe distance: %.2f cm", sensor.min_safe_distance);
    ESP_LOGI(TAG, "Maximum safe distance: %.2f cm", sensor.max_safe_distance);
}

bool sensor_detect_intrusion()
{
    int num_out_of_range_distances = 0;

    for (int i = 0; i < SENSING_NUM_SAMPLES; i++)
    {
        double distance_cm = sensor_get_distance_in_cm();

        // Ignore invalid samples
        if (distance_cm == -1.0)
            continue;

        if (distance_cm < sensor.min_safe_distance)
            num_out_of_range_distances++;
        else if (distance_cm > sensor.max_safe_distance)
            num_out_of_range_distances++;

        if (num_out_of_range_distances >= MAX_OUT_OF_RANGE_DISTANCES)
            return true;

        vTaskDelay(SENSOR_TIMEOUT_MS);
    }

    return false;
}

double sensor_get_distance_in_cm()
{
    uint32_t ticks;

    // Generate Trig pulse
    gpio_set_level(PIN_TRIG, 1);
    esp_rom_delay_us(TRIG_PERIOD_US);
    gpio_set_level(PIN_TRIG, 0);

    // Wait for end of Echo signal
    if (xTaskNotifyWait(0, UINT32_MAX, &ticks, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        // Get the width of the Echo pulse in microseconds
        double pulse_width_us = ((double) ticks) / ((double) esp_clk_apb_freq()) * 1000000.0;
        double distance_cm = pulse_width_us / SOUND_US_PER_CM;

        ESP_LOGI(TAG, "Distance: %.2f cm", distance_cm);
        return distance_cm;
    }

    return -1.0;
}
