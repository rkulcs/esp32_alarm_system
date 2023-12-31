#pragma once

#include <unistd.h>
#include "driver/i2c.h"

void i2c_init();
void lcd_init();

void lcd_write_str(char* str);
void lcd_clear();
void lcd_set_cursor(int y, int x);
void lcd_write_one_line(char* line);
void lcd_write_two_lines(char* line1, char* line2);
