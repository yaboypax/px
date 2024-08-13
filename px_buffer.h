#include <math.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>

#include "px_vector.h"


#define MAX_CHANNELS 4

typedef struct 
{
    int num_samples;
    int num_channels;

    px_fvector vector[MAX_CHANNELS];
    bool is_filled;
} px_float_buffer;

#define BUFFER_TYPE px_float_buffer

static void px_buffer_init(BUFFER_TYPE* buffer, int num_samples, int num_channels)
{
    if (!buffer)
        return;
    
    buffer->num_samples = num_samples;
    buffer->num_channels = num_channels;

    for (int channel = 0; channel < num_channels; ++channel)
    {
        buffer->vector[channel].capacity = 0;
        buffer->vector[channel].size = 0;
        buffer->vector[channel].data = NULL;

        for (int i = 0; i < num_samples; ++i)
        {
            px_fvector_push(&buffer->vector[channel], 1.f);
        }
    }

    buffer->is_filled = false;
}

static void px_buffer_set_sample(BUFFER_TYPE* buffer, int channel, int sample_position, float value)
{
    if ( channel > buffer->num_channels || sample_position > buffer->num_samples)
    {
        printf("OUT OF BUFFER RANGE");
        return;
    }

    buffer->vector[channel].data[sample_position] = value;

}

static float* px_buffer_get_sample(BUFFER_TYPE* buffer, int channel, int sample_position)
{
    if ( channel > buffer->num_channels || sample_position > buffer->num_samples)
    {
        printf("OUT OF BUFFER RANGE");
        return NULL;
    }

    return &buffer->vector[channel].data[sample_position];
}

static void px_buffer_gainf(BUFFER_TYPE* buffer, float in_gain)
{

    assert(buffer);

    for (int channel = 0; channel < buffer->num_channels; ++channel)
    {
        for (int i = 0; i < buffer->num_samples; ++i)
        {
            buffer->vector[channel].data[i] * in_gain;
        }
    }
}


