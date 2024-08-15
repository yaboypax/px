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


// --------------------------------------------------------------------------------------------------------


static px_buffer* px_buffer_create(int num_samples, int num_channels);
static void px_buffer_destroy(px_buffer* buffer);
static void px_buffer_initialize(px_buffer* buffer, int num_samples, int num_channels);

static void px_buffer_set_sample(px_buffer* buffer, int channel, int sample_position, BUFFER_TYPE value);
static BUFFER_TYPE px_buffer_get_sample(px_buffer* buffer, int channel, int sample_position);
static void px_buffer_gain(px_buffer* buffer, BUFFER_TYPE in_gain);

// --------------------------------------------------------------------------------------------------------

static px_buffer* px_buffer_create(int num_samples, int num_channels)
{
    px_buffer* buffer = (px_buffer*)malloc(sizeof(px_buffer));
    px_buffer_initialize(buffer, num_samples, num_channels);
    return buffer;
}

static void px_buffer_destroy(px_buffer* buffer)
{
    if (buffer)
    	free(buffer);
}

static void px_buffer_initialize(px_buffer* buffer, int num_samples, int num_channels)
{
    assert(buffer);
    
    buffer->num_samples = num_samples;
    buffer->num_channels = num_channels;

    for (int channel = 0; channel < num_channels; ++channel)
    {
        px_vector_initialize(&buffer->vector[channel]);

        for (int i = 0; i < num_samples; ++i)
        {
         
	    BUFFER_TYPE* ptr = (BUFFER_TYPE*)malloc(sizeof(BUFFER_TYPE));
            *ptr = 0.f;
	    px_vector_push(&buffer->vector[channel], ptr);
        }
    }

    //buffer->is_filled = false;
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


