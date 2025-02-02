#include "px_globals.h"
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

#ifdef PX_FLOAT_BUFFER
	#define BUFFER_TYPE float
#elif PX_DOUBLE_BUFFER
	#define BUFFER_TYPE double
#endif

typedef struct 
{
    int num_samples;
    int num_channels;

    BUFFER_TYPE* data[MAX_CHANNELS];
    bool is_filled;
} px_buffer;

typedef struct
{
	int num_samples;
	int num_channels;
	BUFFER_TYPE* data;
	bool is_filled;
} px_interleaved_buffer;

// --------------------------------------------------------------------------------------------------------
static px_buffer* px_buffer_create(int num_channels, int num_samples);
static void px_buffer_destroy(px_buffer* buffer);
static void px_buffer_initialize(px_buffer* buffer, int num_channels, int num_samples);
static void px_buffer_clear(px_buffer* buffer);

static void px_buffer_set_sample(px_buffer* buffer, int channel, int sample_position, BUFFER_TYPE value);
static BUFFER_TYPE px_buffer_get_sample(px_buffer* buffer, int channel, int sample_position);
static BUFFER_TYPE* px_buffer_get_pointer(px_buffer* buffer, int channel);

static px_interleaved_buffer* px_buffer_to_interleaved(const px_buffer* src); 

// --------------------------------------------------------------------------------------------------------

static px_buffer* px_buffer_create(int num_channels, int num_samples)
{
    px_buffer* buffer = (px_buffer*)px_malloc(sizeof(px_buffer));
    px_buffer_initialize(buffer, num_samples, num_channels);
    return buffer;
}

static void px_buffer_clear(px_buffer* buffer)
{
	if (buffer)
	{
        for (int channel = 0; channel < buffer->num_channels; ++channel)
        {
            if (buffer->data[channel])
            {
                px_free(buffer->data[channel]);
            }
        }
	}
}

static void px_buffer_destroy(px_buffer* buffer)
{
    if (buffer)
    {
		for (int channel =0; channel < buffer->num_channels; ++channel)
		{
			px_free(buffer->data[channel]);
		}
        px_free(buffer);
    }
}

static void px_buffer_initialize(px_buffer* buffer, int num_channels, int num_samples)
{
    assert(buffer);
    
    buffer->num_samples = num_samples;
    buffer->num_channels = num_channels;
	buffer->is_filled = false;
    for (int channel = 0; channel < num_channels; ++channel)
    {
        buffer->data[channel] = (BUFFER_TYPE*)px_malloc(num_samples * sizeof(BUFFER_TYPE));
    	assert(buffer->data[channel]);
	}
}

static void px_buffer_set_sample(px_buffer* buffer, int channel, int sample_position, BUFFER_TYPE value)
{
	assert(buffer);
    if ( channel >= buffer->num_channels || sample_position >= buffer->num_samples)
    {
        printf("OUT OF BUFFER RANGE");
        return;
    }
    buffer->data[channel][sample_position] = value;
	
	if (!buffer->is_filled)
		buffer->is_filled = true;
}

static BUFFER_TYPE px_buffer_get_sample(px_buffer* buffer, int channel, int sample_position)
{
	if (!buffer->is_filled)
	{
		printf("BUFFER EMPTY");
		return (BUFFER_TYPE)0;
	}
    if ( channel > buffer->num_channels || sample_position > buffer->num_samples)
    {
        printf("OUT OF BUFFER RANGE");
        return (BUFFER_TYPE)0;
    }
	
	return buffer->data[channel][sample_position];
}

static BUFFER_TYPE* px_buffer_get_write_pointer(px_buffer* buffer, int channel)
{

	if ( channel > buffer->num_channels)
    {
        printf("OUT OF BUFFER RANGE");
        return NULL;
    }
    
    return buffer->data[channel];
   
}

static px_interleaved_buffer* px_buffer_to_interleaved(const px_buffer* src) 
{
	px_interleaved_buffer* interleaved_buffer = (px_interleaved_buffer*)malloc(sizeof(px_interleaved_buffer));
	if (!interleaved_buffer) return NULL;
	
	interleaved_buffer->num_samples = src->num_samples;
    interleaved_buffer->num_channels = src->num_channels;
    interleaved_buffer->is_filled = src->is_filled;
	
	size_t total_samples = src->num_samples * src->num_channels;
	interleaved_buffer->data = (BUFFER_TYPE*)malloc(total_samples * sizeof(BUFFER_TYPE));
	if (!interleaved_buffer->data)
	{
		free(interleaved_buffer);
		return NULL;
	}

	for (int sample = 0; sample < src->num_samples; ++sample) {
		for (int channel = 0; channel < src->num_channels; ++channel) {
			int interleaved_index = sample * src->num_channels + channel;
			interleaved_buffer->data[interleaved_index] = src->data[channel][sample];
        }
    }

	return interleaved_buffer;

}




typedef struct {
    float* data;
    int head;
    int tail;
    int max_length;
} px_circular_buffer;


static void px_circular_push(px_circular_buffer* buffer, float value)
{
    int next = (buffer->head + 1) % buffer->max_length;

    if (next == buffer->tail)
        buffer->tail = (buffer->tail + 1) % buffer->max_length;


    buffer->data[buffer->head] = value;
    buffer->head = next;
}

static float px_circular_pop(px_circular_buffer* buffer)
{
    assert(buffer->head != buffer->tail); // Buffer is not empty

    float value = buffer->data[buffer->tail];
    int next = buffer->tail + 1;
    if (next >= buffer->max_length)
        next = 0;

    buffer->tail = next;
    return value;
}

static float px_circular_get_sample(px_circular_buffer* buffer, size_t index)
{
    assert(index >= 0 && index < buffer->max_length);
    return buffer->data[index % buffer->max_length];
}

static void px_circular_initialize(px_circular_buffer* buffer, int max_length)
{
    assert(max_length > 0);

    buffer->data = (float*)px_malloc(sizeof(float) * max_length);
    memset(buffer->data, 0, sizeof(float) * max_length);

    buffer->head = 0;
    buffer->tail = 0;
    buffer->max_length = max_length;
}
static void px_circular_resize(px_circular_buffer* buffer, int new_size)
{
    float* new_data = (float*)px_malloc(sizeof(float) * new_size);
    
    int i = 0;
    int j = buffer->head;

    while (i < buffer->max_length) {
	new_data[i] = buffer->data[j];
	j = (j+1) % buffer->max_length;
	i++;
    }

    px_free(buffer->data);
    buffer->data = new_data;
    buffer->head = 0;
    buffer->tail = buffer->max_length-1;
    buffer->max_length = new_size;

}

#endif
