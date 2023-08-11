#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define BUFFER_SIZE 32

typedef struct
{
    int first;
    int last;
    SemaphoreHandle_t empty_slots;
    SemaphoreHandle_t filled_slots;
    SemaphoreHandle_t mutex;
    double values[BUFFER_SIZE];
} CircularBuffer;

extern CircularBuffer distance_buffer;

void init_circular_buffer(CircularBuffer*);
void insert_value(CircularBuffer*, double);
double remove_value(CircularBuffer*);
