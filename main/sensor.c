#include "sensor.h"

#include "driver/gpio.h"
#include "driver/mcpwm_cap.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const int PIN_TRIG = 13;
static const int PIN_ECHO = 14;

static const int TIMER_CONF_GROUP_ID = 0;
static const int INPUT_SIGNAL_PRESCALE = 1;
static const uint64_t TRIG_PIN_BITMASK = 1ULL << PIN_TRIG;

struct
{
    mcpwm_cap_timer_handle_t timer;
    mcpwm_capture_timer_config_t timer_conf;
    mcpwm_cap_channel_handle_t channel;
    mcpwm_capture_channel_config_t channel_conf;
    mcpwm_capture_event_callbacks_t callbacks;
    gpio_config_t gpio_conf;
} sensor;

bool sensor_callback(mcpwm_cap_channel_handle_t channel, 
                     mcpwm_capture_event_data_t* event_data, void* data)
{
    return false;
}

void set_up_sensor()
{
    // Set up capture timer
    sensor.timer_conf.clk_src = MCPWM_CAPTURE_CLK_SRC_DEFAULT;
    sensor.timer_conf.group_id = TIMER_CONF_GROUP_ID;
    mcpwm_new_capture_timer(&(sensor.timer_conf), &(sensor.timer));

    // Set up capture channel
    sensor.channel_conf.gpio_num = PIN_ECHO;
    sensor.channel_conf.prescale = INPUT_SIGNAL_PRESCALE;
    sensor.channel_conf.flags.neg_edge = true;
    sensor.channel_conf.flags.pos_edge = true;
    sensor.channel_conf.flags.pull_up = true;
    mcpwm_new_capture_channel(sensor.timer, &(sensor.channel_conf), &(sensor.channel));

    // Register capture callback function
    sensor.callbacks.on_cap = sensor_callback;
    mcpwm_capture_channel_register_event_callbacks(sensor.channel, &(sensor.callbacks), 
        xTaskGetCurrentTaskHandle());
    mcpwm_capture_channel_enable(sensor.channel);

    // Configure Trig pin
    sensor.gpio_conf.mode = GPIO_MODE_OUTPUT;
    sensor.gpio_conf.pin_bit_mask = TRIG_PIN_BITMASK;
    gpio_config(&(sensor.gpio_conf));
    gpio_set_level(PIN_TRIG, 0);

    // Set up capture timer
    mcpwm_capture_timer_enable(sensor.timer);
    mcpwm_capture_timer_start(sensor.timer);
}
