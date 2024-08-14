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

    px_vector vector[MAX_CHANNELS];
    bool is_filled;
} px_buffer;

#ifdef PX_FLOAT_BUFFER
	#define BUFFER_TYPE float
#elif PX_DOUBLE_BUFFER
	#define BUFFER_TYPE double
#endif


tatic void px_buffer_init(px_buffer* buffer, int num_samples, int num_channels)
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
            BUFFER_TYPE value = 1.f;
	    BUFFER_TYPE* ptr = &value;
	    px_vector_push(&buffer->vector[channel], ptr);
        }
    }

    buffer->is_filled = false;
}

static void px_buffer_set_sample(px_buffer* buffer, int channel, int sample_position, BUFFER_TYPE value)
{
    if ( channel > buffer->num_channels || sample_position > buffer->num_samples)
    {
        printf("OUT OF BUFFER RANGE");
        return;
    }

    BUFFER_TYPE* ptr = &value;
    buffer->vector[channel].data[sample_position] = ptr;

}

static BUFFER_TYPE px_buffer_get_sample(px_buffer* buffer, int channel, int sample_position)
{
    if ( channel > buffer->num_channels || sample_position > buffer->num_samples)
    {
        printf("OUT OF BUFFER RANGE");
        return 0.f;
    }
    BUFFER_TYPE* ptr = buffer->vector[channel].data[sample_position];
    return *ptr;
}

static void px_buffer_gain(px_buffer* buffer, BUFFER_TYPE in_gain)
{

    assert(buffer);

    for (int channel = 0; channel < buffer->num_channels; ++channel)
    {
        for (int i = 0; i < buffer->num_samples; ++i)
        {
            BUFFER_TYPE* ptr = buffer->vector[channel].data[i];
	    *ptr *= in_gain;
	    buffer->vector[channel].data[i] = ptr;
        }
    }
}


