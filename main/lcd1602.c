/**
 * Functions for using the LCD 1602 display module with I2C. 
 * 
 * Based on:
 * - TINSHARP TC1602B-01 datasheet
 * - ControllersTech ESP32 LCD 1602 library (https://controllerstech.com/i2c-in-esp32-esp-idf-lcd-1602/)
 * - LiquidCrystal I2C library (https://github.com/johnrickman/LiquidCrystal_I2C)
 */

#include "lcd1602.h"

// LCD 1602 parameters
static const int LCD_ADDR = 0x3F;
static const int NUM_ROWS = 2;
static const int NUM_COLS = 16;

static const int PIN_SCL = 22;
static const int PIN_SDA = 21;

// LCD 1602 settings to use
static const int NUM_LINES = 2;
static const int EN_5_BY_10_FONTS = 0;
static const int INCR = 1;
static const int SHIFT = 0;

// Wait periods
static const int POWER_ON_WAIT_US = 55000;
static const int FUNCTION_SET_SECOND_ATTEMPT_WAIT_US = 4300;
static const int FUNCTION_SET_REMAINING_ATTEMPTS_WAIT_US = 130;
static const int POST_FUNCTION_SET_WAIT_US = 2000;

// I2C settings to use
static const int DEFAULT_I2C_PORT = 0;
static const int I2C_CLK_SPEED_HZ = 100000;
static const int DEFAULT_TX_BUF_LEN = 0;
static const int DEFAULT_RX_BUF_LEN = 0;
static const int INTERRUPT_ALLOC_FLAG = 0;

static const int NUM_TRANSFER_DATA_BYTES = 4;
static const int CMD_TICKS_TO_WAIT = 100;

// Commands
static const uint8_t BACKLIGHT_HIGH = 0b00001000;
static const uint8_t EN_HIGH =        0b00000100;
static const uint8_t RW_HIGH =        0b00000010;
static const uint8_t RS_HIGH =        0b00000001;

static const uint8_t CMD_CLEAR =           0b00000001;
static const uint8_t CMD_FUNC_SET =        0b00110000;
static const uint8_t CMD_EN_4BIT =         0b00100000;
static const uint8_t CMD_BASE_LCD_INIT =   0b00100000;
static const uint8_t CMD_DISPLAY_OFF =     0b00001000;
static const uint8_t CMD_DISPLAY_ON =      0b00001100;
static const uint8_t CMD_BASE_ENTRY_MODE = 0b00000100;
static const uint8_t CMD_BASE_SET_CURSOR = 0b10000000;

static const uint8_t MASK_LCD_INIT_2_LINES =         0b00001000;
static const uint8_t MASK_LCD_INIT_5_BY_10_FONTS =   0b00000100;
static const uint8_t MASK_ENTRY_MODE_INCREMENT =     0b00000010;
static const uint8_t MASK_ENTRY_MODE_SHIFT_DISPLAY = 0b00000001;

i2c_config_t config;

void lcd_write_cmd(uint8_t cmd);
void lcd_write_data(uint8_t data);
void transform_data(uint8_t* data_to_send, uint8_t raw_data, uint8_t rw, uint8_t rs);

void i2c_init()
{
    config.mode = I2C_MODE_MASTER;
    config.sda_io_num = PIN_SDA;
    config.scl_io_num = PIN_SCL;
    config.sda_pullup_en = GPIO_PULLUP_ENABLE;
    config.scl_pullup_en = GPIO_PULLUP_ENABLE;
    config.master.clk_speed = I2C_CLK_SPEED_HZ;

    ESP_ERROR_CHECK(i2c_param_config(DEFAULT_I2C_PORT, &config));
    ESP_ERROR_CHECK(i2c_driver_install(DEFAULT_I2C_PORT, config.mode, 
            DEFAULT_TX_BUF_LEN, DEFAULT_RX_BUF_LEN, INTERRUPT_ALLOC_FLAG));
}

void lcd_init()
{
    usleep(POWER_ON_WAIT_US);

    /* Write function set commands after the LCD is turned on, as specified by 
       the datasheet */
    lcd_write_cmd(CMD_FUNC_SET);
    usleep(FUNCTION_SET_SECOND_ATTEMPT_WAIT_US);
    lcd_write_cmd(CMD_FUNC_SET);
    usleep(FUNCTION_SET_REMAINING_ATTEMPTS_WAIT_US);
    lcd_write_cmd(CMD_FUNC_SET);
    usleep(FUNCTION_SET_REMAINING_ATTEMPTS_WAIT_US);

    // Enable 4-bit command mode
    lcd_write_cmd(CMD_EN_4BIT);
    usleep(FUNCTION_SET_REMAINING_ATTEMPTS_WAIT_US);

    // Configure the LCD initialization command
    uint8_t lcd_init_cmd = CMD_BASE_LCD_INIT;

    if (NUM_LINES == 2)
        lcd_init_cmd |= MASK_LCD_INIT_2_LINES;

    if (EN_5_BY_10_FONTS)
        lcd_init_cmd |= MASK_LCD_INIT_5_BY_10_FONTS;

    // Initialize the LCD using the specified settings
    lcd_write_cmd(lcd_init_cmd);
    usleep(POST_FUNCTION_SET_WAIT_US);

    // Turn off and clear the display
    lcd_write_cmd(CMD_DISPLAY_OFF);
    usleep(POST_FUNCTION_SET_WAIT_US);
    lcd_clear();
    usleep(POST_FUNCTION_SET_WAIT_US);

    // Configure the LCD entry mode command
    uint8_t entry_mode_cmd = CMD_BASE_ENTRY_MODE;

    if (INCR)
        entry_mode_cmd |= MASK_ENTRY_MODE_INCREMENT;

    if (SHIFT)
        entry_mode_cmd |= MASK_ENTRY_MODE_SHIFT_DISPLAY;

    // Set the entry mode, then turn on the display
    lcd_write_cmd(entry_mode_cmd);
    usleep(POST_FUNCTION_SET_WAIT_US);
    lcd_write_cmd(CMD_DISPLAY_ON);
}

void lcd_write_cmd(uint8_t cmd)
{
    uint8_t transferrable_data[NUM_TRANSFER_DATA_BYTES];
    transform_data(transferrable_data, cmd, 0, 0);

    ESP_ERROR_CHECK(i2c_master_write_to_device(DEFAULT_I2C_PORT, LCD_ADDR, 
            transferrable_data, NUM_TRANSFER_DATA_BYTES, CMD_TICKS_TO_WAIT));
}

void lcd_write_data(uint8_t data)
{
    uint8_t transferrable_data[NUM_TRANSFER_DATA_BYTES];
    transform_data(transferrable_data, data, 0, 1);

    ESP_ERROR_CHECK(i2c_master_write_to_device(DEFAULT_I2C_PORT, LCD_ADDR, 
            transferrable_data, NUM_TRANSFER_DATA_BYTES, CMD_TICKS_TO_WAIT));
}

void lcd_write_str(char* str)
{
    int i = 0;

    while (str[i] != '\0')
    {
        lcd_write_data(str[i]);
        i++;
    }
}

void lcd_clear()
{
    uint8_t transferrable_data[NUM_TRANSFER_DATA_BYTES];
    transform_data(transferrable_data, CMD_CLEAR, 0, 0);

    ESP_ERROR_CHECK(i2c_master_write_to_device(DEFAULT_I2C_PORT, LCD_ADDR, 
            transferrable_data, NUM_TRANSFER_DATA_BYTES, CMD_TICKS_TO_WAIT));
}

void lcd_set_cursor(int y, int x)
{
    ESP_ERROR_CHECK((y < 0 || y >= NUM_ROWS) || (x < 0 || x >= NUM_COLS));

    uint8_t set_cursor_cmd = CMD_BASE_SET_CURSOR;
    set_cursor_cmd |= (y << 6) | x;
    lcd_write_cmd(set_cursor_cmd);
}

/**
 * Transforms an 8-bit command or data byte into four bytes which can be written to the
 * LCD in 4-bit mode.
 * 
 * @param data_to_send An array of bytes which can be written to the LCD
 * @param raw_data The original data to transform
 * @param rw R/W bit
 * @param rs Reset bit
 */
void transform_data(uint8_t* data_to_send, uint8_t raw_data, uint8_t rw, uint8_t rs)
{
    // Configure metadata in the four LSB using the backlight, R/W, and Reset settings
    uint8_t metadata = 0b00000000 | BACKLIGHT_HIGH;

    if (rw == 1)
        metadata |= RW_HIGH;

    if (rs == 1)
        metadata |= RS_HIGH;

    // Split the raw byte into two nibbles, and append the metadata nibble to them
    uint8_t upper_nibble = (raw_data & 0b11110000) | metadata;
    uint8_t lower_nibble = (raw_data << 4) | metadata;

    // Set up the sequence of bytes to write to the LCD
    data_to_send[0] = upper_nibble | EN_HIGH;
    data_to_send[1] = upper_nibble;
    data_to_send[2] = lower_nibble | EN_HIGH;
    data_to_send[3] = lower_nibble;
}
