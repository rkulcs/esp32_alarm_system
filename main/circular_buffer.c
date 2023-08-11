#include "circular_buffer.h"

static const int SEM_TICKS_TO_WAIT = 10;

CircularBuffer distance_buffer;

void init_circular_buffer(CircularBuffer* buffer)
{
    buffer->first = -1;
    buffer->last = -1;
    buffer->empty_slots = xSemaphoreCreateCounting(BUFFER_SIZE, BUFFER_SIZE);
    buffer->filled_slots = xSemaphoreCreateCounting(BUFFER_SIZE, 0);
    buffer->mutex = xSemaphoreCreateMutex();
}

void insert_value(CircularBuffer* buffer, double value)
{
    xSemaphoreTake(buffer->empty_slots, SEM_TICKS_TO_WAIT);
    xSemaphoreTake(buffer->mutex, SEM_TICKS_TO_WAIT);

    if (buffer->first == -1)
        buffer->first = 0;

    buffer->last = (buffer->last + 1) % BUFFER_SIZE;
    buffer->values[buffer->last] = value;
    xSemaphoreGive(buffer->filled_slots);

    xSemaphoreGive(buffer->mutex);
}

double remove_value(CircularBuffer* buffer)
{
    xSemaphoreTake(buffer->filled_slots, SEM_TICKS_TO_WAIT);
    xSemaphoreTake(buffer->mutex, SEM_TICKS_TO_WAIT);

    double value = buffer->values[buffer->first];
    buffer->first = (buffer->first + 1) % BUFFER_SIZE;
    xSemaphoreGive(buffer->empty_slots);

    xSemaphoreGive(buffer->mutex);

    return value;
}
