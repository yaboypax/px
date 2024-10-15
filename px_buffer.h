#include <math.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>

#include "px_vector.h"
#include "px_memory.h"

#ifndef PX_BUFFER_H
#define PX_BUFFER_H

/*

  Somewhat type generic (double/float) buffer for working with px_. Includes internal array of type generic (void*) px_vector.

  //include

  #define PX_FLOAT_BUFFER
  #include "px_buffer.h"

  	or:

  #define PX_DOUBLE_BUFFER
  #include "px_buffer.h"



	//init

	stack initialize:
		
		px_buffer buffer;
		px_buffer_initialize(&buffer, (int)num_samples, (int)num_channels);
	heap:
		px_buffer* buffer = px_buffer_create( (int)num_samples, (int)num_channels);
		// initialize called within create()
	free:
		px_buffer_destroy(buffer);



	// use
	

	get_sample:

		for (int i = 0; i < buffer.num_samples; ++i) 
		{
			float leftValue = px_buffer_get_sample(&buffer, 0, i);
			float rightValue = px_buffer_get_sample(&buffer, 1, i);
		}


		for (int channel = 0; i < buffer.num_channels; ++channel)
		{	
			for (int sample = 0; i < buffer.num_samples; ++sample) 
			{
				float value = px_buffer_get_sample(&buffer, channel, i);
			}
		}




	get_pointer:

		for (int i = 0; i < buffer.num_samples; ++i) 
		{
			float* left_value = px_buffer_get_pointer(&buffer, 0, i);
			float* right_value = px_buffer_get_pointer(&buffer, 1, i);
		}


		for (int channel = 0; i < buffer.num_channels; ++channel)
		{	
			for (int sample = 0; i < buffer.num_samples; ++sample) 
			{
				float* value = px_buffer_get_pointer(&buffer, channel, i);
			}
		}



*/

#ifndef MAX_CHANNELS
	#define MAX_CHANNELS 4
#endif

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
static BUFFER_TYPE* px_buffer_get_pointer(px_buffer* buffer, int channel, int sample_position);

static void px_buffer_gain(px_buffer* buffer, BUFFER_TYPE in_gain);

// --------------------------------------------------------------------------------------------------------

static px_buffer* px_buffer_create(int num_samples, int num_channels)
{
    px_buffer* buffer = (px_buffer*)px_malloc(sizeof(px_buffer));
    px_buffer_initialize(buffer, num_samples, num_channels);
    return buffer;
}

static void px_buffer_destroy(px_buffer* buffer)
{
    if (buffer)
    {
        for (int channel = 0; channel < buffer->num_channels; ++channel)
        {
            if (buffer->vector[channel].data)
            {
                px_free(buffer->vector[channel].data);
                buffer->vector[channel].data = NULL;
            }
        }
        px_free(buffer);
    }
}

static void px_buffer_initialize(px_buffer* buffer, int num_samples, int num_channels)
{
    assert(buffer);
    
    buffer->num_samples = num_samples;
    buffer->num_channels = num_channels;

    for (int channel = 0; channel < num_channels; ++channel)
    {
        px_vector_initialize(&buffer->vector[channel]);
	buffer->vector[channel].data = (void**)px_malloc(num_samples * sizeof(BUFFER_TYPE*));
    }
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

static BUFFER_TYPE* px_buffer_get_pointer(px_buffer* buffer, int channel, int sample_position)
{
    if ( channel > buffer->num_channels || sample_position > buffer->num_samples)
    {
        printf("OUT OF BUFFER RANGE");
        return NULL;
    }
    
    BUFFER_TYPE* ptr = buffer->vector[channel].data[sample_position];
    
    if (ptr)
	return ptr;
    else
	return NULL;
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

#endif
