#include <math.h>
#include <assert.h>
#include <stdarg.h>

#ifndef PX_GLOBALS_H
#define PX_GLOBALS_H

// Math Constants
// ---------------------------------
#define DC_OFFSET 1.0E-25
#define PI 3.141592653589793
#define sgn(val) ((0 < val) - (val < 0))
// ---------------------------------
// Gain

// linear -> dB conversion
static inline float lin2dB(float lin) {
    static const float LOG_2_DB = 8.6858896380650365530225783783321;	// 20 / ln( 10 )
    return log(lin) * LOG_2_DB;
}

// dB -> linear conversion
static inline float dB2lin(float dB) {
    static const float DB_2_LOG = 0.11512925464970228420089957273422;	// ln( 10 ) / 20
    return exp(dB * DB_2_LOG);
}
// -------------------------------------
//
// Mid Side
//
//
typedef struct
{
    float left;
    float right;
} px_ms_decoded;

typedef struct
{
    float mid;
    float side;
} px_ms_encoded;


static inline px_ms_encoded px_ms_encode(px_ms_decoded decoded)
{
	px_ms_encoded encoded;
	encoded.mid = 0.5f * (decoded.left + decoded.right);
	encoded.side = 0.5f * (decoded.left - decoded.right);
	return encoded;
}

static inline px_ms_decoded px_ms_decode(px_ms_encoded encoded)
{
	px_ms_decoded decoded;
	decoded.left = encoded.mid + encoded.side;
	decoded.right = encoded.mid - encoded.side;
	return decoded;
}

// assert for process functions
// ------------------------------------------------------------------------------------------------------


	
static void px_assert_mono(void* control_pointer, float* value_pointer)
{
	// check if DSP object passed is NULL or uninitialized
	assert(control_pointer);

	// check if value is valid float pointer
	assert(value_pointer);
}

static void px_assert_stereo(void* control_pointer, float* channel_one_pointer, float* channel_two_pointer)
{
	// check if DSP object passed is NULL or uninitialized
	assert(control_pointer);

	// check if value is valid float pointer
	assert(channel_one_pointer);
	assert(channel_two_pointer);
}

#define EXPAND(x) x
#define GET_ASSERT_MACRO(_1, _2, _3, NAME, ...) NAME

#define px_assert(...) EXPAND(GET_ASSERT_MACRO(__VA_ARGS__, px_assert_stereo, px_assert_mono)(__VA_ARGS__))


#endif 
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#include "px_memory.h"

#ifndef PX_VECTOR_H
#define PX_VECTOR_H
/* -------------------------------------------------------------------------


    type generic vector


   -------------------------------------------------------------------------*/

typedef struct {
    size_t size;
    size_t capacity;
    void** data;
} px_vector;

static px_vector* px_vector_create();
static void px_vector_initialize(px_vector* vector);
static void px_vector_destroy(px_vector* vector);

static void px_vector_push(px_vector* vector, void* value);
static void px_vector_pop(px_vector* vector);

static void* px_vector_get(px_vector* vector, size_t index);
static void px_vector_remove(px_vector* vector, size_t index);

static void px_vector_copy(px_vector* dest_vector, px_vector* source_vector);
static void px_vector_resize(px_vector* vector, const size_t new_size);

// ----------------------------------------------------------------------------

static px_vector* px_vector_create()
{
    px_vector* vector = (px_vector*)px_malloc(sizeof(px_vector));
    px_vector_initialize(vector);
    return vector;
}

static void px_vector_initialize(px_vector* vector)
{
    assert(vector);

    vector->data = NULL;
    vector->size = 0;
    vector->capacity = 0;
}

static void px_vector_destroy(px_vector* vector)
{
    if (vector)
    {
        px_free(vector->data);
        px_free(vector);
    }
}

static void px_vector_push(px_vector* vector, void* value)
{
    assert(vector);

    if (vector->size == vector->capacity) 
    {
        size_t new_capacity = (vector->capacity == 0) ? 1 : vector->capacity * 2;
        void** new_data = (void**)realloc(vector->data, sizeof(void*) * new_capacity);
        if (new_data)
	{
            vector->data = new_data;
            vector->capacity = new_capacity;
        }
    }

    vector->data[vector->size] = value;
    vector->size++;
}

static void px_vector_pop(px_vector* vector)
{
    assert(vector);
    if (vector->size > 0)
    {
        vector->size--;
        vector->data[vector->size] = NULL;
    }
}

static void* px_vector_get(px_vector* vector, size_t index)
{
    assert(vector);
    void* ptr;
    
    if (index < vector->size)
    {
        ptr = vector->data[index];
    }
    else
    {
	printf("OUT OF RANGE");
        ptr = NULL;	
    }
    
    return ptr;
}

static void px_vector_remove(px_vector* vector , size_t index)
{
    assert(vector);
    if (index < vector->size)
    {
	vector->data[index] = NULL;
	for (size_t i = index; i < (vector->size - 1); ++i)
    	{
	    vector->data[i] = vector->data[i+1];
	}
	vector->size--;
    }
    else
    {
	printf("OUT OF RANGE");
    }    

}

static void px_vector_copy(px_vector* dest_vector, px_vector* source_vector)
{
    assert(dest_vector && source_vector);

    dest_vector->size = source_vector->size;
    dest_vector->capacity = source_vector->capacity;

    void** new_data = (void**)px_malloc(sizeof(void*) * dest_vector->capacity);
    if (new_data) 
    {
        memcpy(new_data, source_vector->data, sizeof(void*) * source_vector->size);
        dest_vector->data = new_data;
    }
}

static void px_vector_resize(px_vector* vector, const size_t new_size)
{
    if (!vector)
        return;
    if (new_size == vector->size)
        return;

    if (new_size > vector->capacity)
    {
        size_t new_capacity = new_size * 2;
        void** new_data = (void**)realloc(vector->data, sizeof(void*) * new_capacity);
        if (new_data)
	{
            vector->data = new_data;
            vector->capacity = new_capacity;
        }
    }

    vector->size = new_size;
}

#endif#include <math.h>
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
