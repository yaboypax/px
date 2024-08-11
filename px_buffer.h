#include <math.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>

#include "px_vector.h"


#define MAX_CHANNELS 4

typedef struct 
{
    int numSamples;
    int numChannels;

    px_fvector vector[MAX_CHANNELS];
    bool isFilled;
} px_float_buffer;

#define BUFFER_TYPE px_float_buffer

static void px_buffer_init(BUFFER_TYPE* buffer, int numSamples, int numChannels)
{
    if (!buffer)
        return;
    
    buffer->numSamples = numSamples;
    buffer->numChannels = numChannels;

    for (int channel = 0; channel < numChannels; ++channel)
    {
        buffer->vector[channel].capacity = 0;
        buffer->vector[channel].size = 0;
        buffer->vector[channel].data = NULL;

        for (int i = 0; i < numSamples; ++i)
        {
            px_fvector_push(&buffer->vector[channel], 1.f);
        }
    }

    buffer->isFilled = false;
}

static void px_buffer_set_sample(BUFFER_TYPE* buffer, int channel, int samplePosition, float value)
{
    if ( channel > buffer->numChannels || samplePosition > buffer->numSamples)
    {
        printf("OUT OF BUFFER RANGE");
        return;
    }

    buffer->vector[channel].data[samplePosition] = value;

}

static float* px_buffer_get_sample(BUFFER_TYPE* buffer, int channel, int samplePosition)
{
    if ( channel > buffer->numChannels || samplePosition > buffer->numSamples)
    {
        printf("OUT OF BUFFER RANGE");
        return NULL;
    }

    return &buffer->vector[channel].data[samplePosition];
}

static void px_buffer_gainf(BUFFER_TYPE* buffer, float inGain)
{

    assert(buffer);

    for (int channel = 0; channel < buffer->numChannels; ++channel)
    {
        for (int i = 0; i < buffer->numSamples; ++i)
        {
            buffer->vector[channel].data[i] * inGain;
        }
    }
}


