#include <stdarg.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>



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

	typedef enum
	{
		BOTH = 0,
		LEFT,
		RIGHT,
		MID,
		SIDE
	} CHANNEL_FLAG;


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

#define px_map(value, inputMin, inputMax, outputMin, outputMax) \
		      ((inputMin > inputMax) ? (outputMin) : 		\
			  ((outputMin) + ((outputMax) - (outputMin)) * (((value) - (inputMin)) / ((inputMax) - (inputMin)))))

#endif 
#ifndef PX_MEMORY_H
#define PX_MEMORY_H


#define px_malloc(a) \
	malloc(a);   \
	printf("malloc at %s line: %d in %s \n", __func__, __LINE__, __FILE__); \

#define px_free(a) \
	free(a);   \
	printf("free at %s line: %d in %s \n", __func__, __LINE__, __FILE__); \


#endif
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

#endif
#ifndef PX_BUFFER_H
#define PX_BUFFER_H

/*

  Somewhat type generic (double/float) buffer for working with px_. Includes internal array of type generic (void*) px_vector.

  //include

  #define PX_FLOAT_BUFFER

  	or:

  #define PX_DOUBLE_BUFFER



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
    BUFFER_TYPE* ptr = (BUFFER_TYPE*)buffer->vector[channel].data[sample_position];
    return *ptr;
}

static BUFFER_TYPE* px_buffer_get_pointer(px_buffer* buffer, int channel, int sample_position)
{
    if ( channel > buffer->num_channels || sample_position > buffer->num_samples)
    {
        printf("OUT OF BUFFER RANGE");
        return NULL;
    }
    
    BUFFER_TYPE* ptr = (BUFFER_TYPE*)buffer->vector[channel].data[sample_position];
    
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
            BUFFER_TYPE* ptr = (float*)buffer->vector[channel].data[i];
	    *ptr *= in_gain;
	    buffer->vector[channel].data[i] = ptr;
        }
    }
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
#ifndef PX_DELAY_H
#define PX_DELAY_H

typedef struct
{
    float seconds;

    // split for sample interpolation
    float fraction;
    int whole;
} delay_time;

typedef struct
{
   float sample_rate;
   float feedback;
   delay_time time;
   float max_time; // in seconds
   float dry_wet;
} px_delay_parameters;

typedef struct
{
    px_delay_parameters parameters;
    px_circular_buffer buffer;

} px_delay_line;


typedef struct
{
	bool ping_pong;
    px_delay_line left;
    px_delay_line right;
} px_stereo_delay;

static px_delay_line* px_create_mono_delay(float sample_rate, float max_time);
static void px_destroy_mono_delay(px_delay_line* delay);
static void px_delay_mono_free_buffer(px_delay_line* delay);

static void px_delay_mono_initialize(px_delay_line* delay, float sample_rate, float max_time);
static void px_delay_mono_prepare(px_delay_line* delay, float sample_rate);
static void px_delay_mono_set_time(px_delay_line* delay, float time);
static void px_delay_mono_set_feedback(px_delay_line* delay, float feedback);
static void px_delay_mono_process(px_delay_line* delay, float* input);

static px_stereo_delay* px_create_stereo_delay(float sample_rate, float max_time, bool ping_pong);
static void px_destroy_stereo_delay(px_stereo_delay* delay);
static void px_delay_stereo_free_buffer(px_stereo_delay* delay);

static void px_delay_stereo_initialize(px_stereo_delay* delay, float sample_rate, float max_time, bool ping_pong);
static void px_delay_stereo_prepare(px_stereo_delay* delay, float sample_rate);
static void px_delay_stereo_set_time(px_stereo_delay* delay, float time, CHANNEL_FLAG channel);
static void px_delay_stereo_set_feedback(px_stereo_delay* delay, float feedback, CHANNEL_FLAG channel);
static void px_delay_stereo_set_ping_pong(px_stereo_delay* delay, bool ping_pong);
static void px_delay_stereo_process(px_stereo_delay* delay, float* input_left, float* input_right);


static px_delay_line* px_create_mono_delay(float sample_rate, float max_time)
{
	px_delay_line* delay = (px_delay_line*) px_malloc(sizeof(px_delay_line));
	assert(delay);

	px_delay_mono_initialize(delay, sample_rate, max_time);
	return delay;
}

static void px_destroy_mono_delay(px_delay_line* delay)
{
	if (delay)
	{
		px_free(delay->buffer.data);
		px_free(delay);
	}
}

static px_stereo_delay* px_create_stereo_delay(float sample_rate, float max_time, bool ping_pong)
{
	px_stereo_delay* delay = (px_stereo_delay*) px_malloc(sizeof(px_stereo_delay));
	assert(delay);

	px_delay_stereo_initialize(delay, sample_rate, max_time, ping_pong);
	return delay;
}

static void px_destroy_stereo_delay(px_stereo_delay* delay)
{
	if (delay)
	{
		px_free(delay->left.buffer.data);
		px_free(delay->right.buffer.data);
		px_free(delay);
	}
}


static void px_delay_mono_initialize(px_delay_line* delay, float sample_rate, float max_time)
{
   assert(delay);
  
   // TODO REMOVE THIS 
   if (delay->parameters.max_time == max_time)
   {
		px_delay_mono_prepare(delay, sample_rate);
		return;
   }

   delay_time time = { 1.f, 0.f, 1 };
   px_delay_parameters parameters = {  sample_rate, 0.5f, time, max_time, 0.5f };
   delay->parameters = parameters;

   int max_samples = sample_rate * max_time;
   px_circular_initialize(&delay->buffer, max_samples);
}

static void px_delay_stereo_initialize(px_stereo_delay* delay, float sample_rate, float max_time, bool ping_pong)
{
	assert(delay);

	delay_time time = {1.f, 0.f, 1 };
	px_delay_parameters parameters = { sample_rate, 0.5f, time, max_time, 0.5f };
	
	delay->ping_pong = ping_pong;
	delay->left.parameters = parameters;
	delay->right.parameters = parameters;

	int max_samples = sample_rate * max_time;
	px_circular_initialize(&delay->left.buffer, max_samples);
	px_circular_initialize(&delay->right.buffer, max_samples);	
}

static void px_delay_mono_free_buffer(px_delay_line* delay)
{
	if (delay)
	{
		px_free(delay->buffer.data);
	}
}

static void px_delay_stereo_free_buffer(px_stereo_delay* delay)
{
	if (delay)
	{
		px_free(delay->left.buffer.data);
		px_free(delay->right.buffer.data);
	}
}
				
static void px_delay_mono_prepare(px_delay_line* delay, float sample_rate)
{
    assert(delay);
	
	delay->parameters.sample_rate = sample_rate;
	int max_samples = sample_rate * delay->parameters.max_time;
	px_circular_initialize(&delay->buffer, max_samples);
}

static void px_delay_stereo_prepare(px_stereo_delay* delay, float sample_rate)
{
	assert(delay);
	
	delay->left.parameters.sample_rate = sample_rate;
	delay->right.parameters.sample_rate = sample_rate;

	//error with initialization
	assert(delay->left.parameters.max_time == delay->right.parameters.max_time);
	int max_samples = sample_rate * delay->left.parameters.max_time;

	px_circular_initialize(&delay->left.buffer, max_samples);
	px_circular_initialize(&delay->right.buffer, max_samples);
}

static void px_delay_mono_set_time(px_delay_line* delay, float time)
{
   assert(delay);
   assert(time > 0 && time < delay->parameters.max_time);
   delay->parameters.time.seconds = time;
   
   float time_in_samples = delay->parameters.sample_rate * time;
   delay->parameters.time.whole = (int)floorf(time_in_samples);
   delay->parameters.time.fraction = time_in_samples - delay->parameters.time.whole;
}

static void px_delay_stereo_set_time(px_stereo_delay* delay, float time, CHANNEL_FLAG channel)
{
	assert(delay);
	assert(time>0 && time < delay->left.parameters.max_time);
	switch (channel)
	{
			case BOTH:
			{
				px_delay_mono_set_time(&delay->left, time);
				px_delay_mono_set_time(&delay->right, time);
			}
			case LEFT:
			{
				px_delay_mono_set_time(&delay->left, time);
			}	
			case RIGHT:
			{
				px_delay_mono_set_time(&delay->right, time);
			}
	}	

}

static void px_delay_mono_set_feedback(px_delay_line* delay, float feedback)
{
   assert(delay);
   assert(feedback < 1.01f);

   delay->parameters.feedback = feedback;
}

static void px_delay_stereo_set_feedback(px_stereo_delay* delay, float feedback, CHANNEL_FLAG channel)
{
	assert(delay);
	assert(feedback < 1.01f);

	switch (channel)
	{
			case BOTH:
			{
				px_delay_mono_set_feedback(&delay->left, feedback);
				px_delay_mono_set_feedback(&delay->right, feedback);
			}
			case LEFT:
			{
				px_delay_mono_set_feedback(&delay->left, feedback);
			}	
			case RIGHT:
			{
				px_delay_mono_set_feedback(&delay->right, feedback);
			}
	}	
}

static void px_delay_stereo_set_ping_pong(px_stereo_delay* delay, bool ping_pong)
{
	assert(delay);
	delay->ping_pong = ping_pong;
}

static void px_delay_mono_process(px_delay_line* delay, float* input)
{
    px_assert(delay, input);

    int read1 = (delay->buffer.head - delay->parameters.time.whole + delay->buffer.max_length) % delay->buffer.max_length;
    int read2 = (read1 + 1) % delay->buffer.max_length;

    float delayed1 = px_circular_get_sample(&delay->buffer, (size_t) read1);
    float delayed2 = px_circular_get_sample(&delay->buffer, (size_t) read2);
    
    // linear interpolation
    float delayed_interp = delayed1 + delay->parameters.time.fraction * (delayed2 - delayed1);
    
    float feedback = (*input) + (delay->parameters.feedback * delayed_interp);
    px_circular_push(&delay->buffer, feedback);

    float output = ((1.0f - delay->parameters.dry_wet) * (*input)) + (delay->parameters.dry_wet * delayed_interp);
    *input = output;
}

static void px_delay_stereo_process(px_stereo_delay* delay, float* input_left, float* input_right)
{
	px_assert(delay, input_left, input_right);
	
	if (delay->ping_pong)
	{
        int read_left1 = (delay->left.buffer.head - delay->left.parameters.time.whole + delay->left.buffer.max_length) % delay->left.buffer.max_length;
        int read_right1 = (delay->right.buffer.head - (delay->right.parameters.time.whole + (delay->left.parameters.time.whole / 2)) + delay->right.buffer.max_length) % delay->left.buffer.max_length; 
		int read_left2 = (read_left1 + 1) % delay->left.buffer.max_length;
		int read_right2 = (read_right1 + 1) % delay->right.buffer.max_length;

		float delayed_left1 = px_circular_get_sample(&delay->left.buffer, (size_t) read_left1);
		float delayed_left2 = px_circular_get_sample(&delay->left.buffer, (size_t) read_left2);
		float delayed_right1 = px_circular_get_sample(&delay->right.buffer, (size_t) read_right1);
		float delayed_right2 = px_circular_get_sample(&delay->right.buffer, (size_t) read_right2);

		float delayed_interp_left = delayed_left1 + delay->left.parameters.time.fraction * (delayed_left2 - delayed_left1);
		float delayed_interp_right = delayed_right1 + delay->right.parameters.time.fraction * (delayed_right2 - delayed_right1);

    	float feedback_left = (*input_left) + (delay->left.parameters.feedback * delayed_interp_right); // Right feedback to left
    	float feedback_right = (*input_right) + (delay->right.parameters.feedback * delayed_interp_left); // Left feedback to right

    	// Push feedback into respective buffers
    	px_circular_push(&delay->left.buffer, feedback_left);
    	px_circular_push(&delay->right.buffer, feedback_right);

		*input_left = ((1.0f - delay->left.parameters.dry_wet) * (*input_left)) + (delay->left.parameters.dry_wet * delayed_interp_left);
		*input_right = ((1.0f - delay->right.parameters.dry_wet) * (*input_right)) + (delay->right.parameters.dry_wet * delayed_interp_right);
	}
	else
	{
		px_delay_mono_process(&delay->left, input_left);
		px_delay_mono_process(&delay->right, input_right);
	}
}

#endif
#ifndef PX_BIQUAD_H
#define PX_BIQUAD_H

// PX_BIQUAD filter
//
//

typedef enum {
   BIQUAD_NONE,
   BIQUAD_LOWPASS,
   BIQUAD_HIGHPASS,
   BIQUAD_BANDPASS,
   BIQUAD_NOTCH,
   BIQUAD_PEAK,
   BIQUAD_LOWSHELF,
   BIQUAD_HIGHSHELF,
   BIQUAD_LOWSHELF_NOQ,
   BIQUAD_HIGHSHELF_NOQ,
   BIQUAD_ALLPASS
} BIQUAD_FILTER_TYPE;

typedef struct 
{
   float a0;
   float a1;
   float a2;
   float b1;
   float b2;
   float z1;
   float z2;
} px_biquad_coefficients;

typedef struct
{
   float sample_rate;
   float frequency;
   float quality;
   float gain;
   BIQUAD_FILTER_TYPE type;
} px_biquad_parameters;

typedef struct
{
   px_biquad_coefficients coefficients;
   px_biquad_parameters parameters;
} px_biquad;


// api functions
// -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
static px_biquad* px_biquad_create(float sample_rate, BIQUAD_FILTER_TYPE type); 
static void px_biquad_destroy(px_biquad* biquad);

static void px_biquad_process(px_biquad* biquad, float* input);
static void px_biquad_initialize(px_biquad* biquad, float sample_rate, BIQUAD_FILTER_TYPE type);

static void px_biquad_set_frequency(px_biquad* biquad, float in_frequency);
static void px_biquad_set_quality(px_biquad* biquad, float in_quality);
static void px_biquad_set_gain(px_biquad* biquad, float in_gain);
static void px_biquad_set_type(px_biquad* biquad, BIQUAD_FILTER_TYPE in_type);

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// inline functions
// ----------------------------------------------------------------------------------

static inline float px_biquad_filter(px_biquad* biquad, float input);
static inline void px_biquad_update_coefficients(const px_biquad_parameters parameters, px_biquad_coefficients* coefficients);

// ---------------------------------------------------------------------------------------

static void px_biquad_process(px_biquad* biquad, float* input)
{
    px_assert(biquad, input);
    float mono = *input;
    *input = px_biquad_filter(biquad, mono);
}

static void px_biquad_initialize(px_biquad* biquad, float sample_rate, BIQUAD_FILTER_TYPE type)
{
    assert(biquad);
    px_biquad_parameters parameters = { sample_rate, 100.f, 0.5f, 0.f, type };
    px_biquad_coefficients coefficients = { 1.f, 0.f, 0.0f, 0.0f, 0.0f };
    px_biquad_update_coefficients(parameters, &coefficients);

    biquad->parameters = parameters;
    biquad->coefficients = coefficients;
}

static px_biquad* px_biquad_create(float sample_rate, BIQUAD_FILTER_TYPE type)
{
    px_biquad* biquad = (px_biquad*)malloc(sizeof(px_biquad));
    px_biquad_initialize(biquad, sample_rate, type);
    return biquad;
}

static void px_biquad_destroy(px_biquad* biquad)
{
    if (biquad)
	free(biquad);
}

static void px_biquad_set_frequency(px_biquad* biquad, float in_frequency)
{
    assert(biquad);
    biquad->parameters.frequency = in_frequency;
    px_biquad_update_coefficients(biquad->parameters, &biquad->coefficients);
}

static void px_biquad_set_quality(px_biquad* biquad, float in_quality)
{
    assert(biquad);
    if (in_quality > 0.0f)
    {
        biquad->parameters.quality = in_quality;
        px_biquad_update_coefficients(biquad->parameters, &biquad->coefficients);
    }
}

static void px_biquad_set_gain(px_biquad* biquad, float in_gain)
{
    assert(biquad);
    biquad->parameters.gain = in_gain;
    px_biquad_update_coefficients(biquad->parameters, &biquad->coefficients);
}

static void px_biquad_set_type(px_biquad* biquad, BIQUAD_FILTER_TYPE in_type)
{
    assert(biquad);
    biquad->parameters.type = in_type;
    px_biquad_update_coefficients(biquad->parameters, &biquad->coefficients);
}

// ------------------------------------------------------------------------------------------------------------------------------

static inline float px_biquad_filter(px_biquad* biquad, float input)
{
    float out = input * biquad->coefficients.a0 + biquad->coefficients.z1;
    biquad->coefficients.z1 = input * biquad->coefficients.a1 + biquad->coefficients.z2 - biquad->coefficients.b1 * out;
    biquad->coefficients.z2 = input * biquad->coefficients.a2 - biquad->coefficients.b2 * out;
    return (float)out;
}

static inline void px_biquad_update_coefficients(const px_biquad_parameters parameters, px_biquad_coefficients* coefficients)
{
    float a0 = coefficients->a0;
    float a1 = coefficients->a1;
    float a2 = coefficients->a2;
    float b1 = coefficients->b1;
    float b2 = coefficients->b2;

    if (parameters.frequency <= 0.f || parameters.frequency != parameters.frequency)
    {
        a0 = 1.f;
        a1 = 0.f;
        a2 = 0.f;
        b1 = 0.f;
        b2 = 0.f;
        return;
    }

    float norm;
    float V = pow(10.f, fabs(parameters.gain) / 20.0f);
    float K = tan(PI * (parameters.frequency / parameters.sample_rate));

    //  printf("%d", V);
    //  printf("%d", K);

    switch (parameters.type)
    {
    case BIQUAD_LOWPASS:
        norm = 1.f / (1.f + K / parameters.quality + K * K);
        a0 = K * K * norm;
        a1 = 2.f * a0;
        a2 = a0;
        b1 = 2.f * (K * K - 1.f) * norm;
        b2 = (1.f - K / parameters.quality + K * K) * norm;
        break;

    case BIQUAD_HIGHPASS:
        norm = 1.f / (1.f + K / parameters.quality + K * K);
        a0 = 1.f * norm;
        a1 = -2.f * a0;
        a2 = a0;
        b1 = 2.f * (K * K - 1.f) * norm;
        b2 = (1.f - K / parameters.quality + K * K) * norm;
        break;

    case BIQUAD_BANDPASS:
        norm = 1.f / (1.f + K / parameters.quality + K * K);
        a0 = K / parameters.quality * norm;
        a1 = 0.f;
        a2 = -a0;
        b1 = 2.f * (K * K - 1.f) * norm;
        b2 = (1.f - K / parameters.quality + K * K) * norm;
        break;

    case BIQUAD_NOTCH:
        norm = 1.f / (1.f + K / parameters.quality + K * K);
        a0 = (1.f + K * K) * norm;
        a1 = 2.f * (K * K - 1.f) * norm;
        a2 = a0;
        b1 = a1;
        b2 = (1.f - K / parameters.quality + K * K) * norm;
        break;

    case BIQUAD_PEAK:
        if (parameters.gain >= 0.f)
        { // boost
            norm = 1.f / (1.f + K / parameters.quality + K * K);
            a0 = (1.f + K / parameters.quality * V + K * K) * norm;
            a1 = 2.f * (K * K - 1.f) * norm;
            a2 = (1.f - K / parameters.quality * V + K * K) * norm;
            b1 = a1;
            b2 = (1.f - K / parameters.quality + K * K) * norm;
        }
        else
        { // cut
            norm = 1.f / (1.f + K / parameters.quality * V + K * K);
            a0 = (1.f + K / parameters.quality + K * K) * norm;
            a1 = 2.f * (K * K - 1.f) * norm;
            a2 = (1.f - K / parameters.quality + K * K) * norm;
            b1 = a1;
            b2 = (1.f - K / parameters.quality * V + K * K) * norm;
        }
        break;

    case BIQUAD_LOWSHELF:
        if (parameters.gain >= 0.f)
        { // boost
            norm = 1.f / (1.f + K / parameters.quality + K * K);
            a0 = (1.f + sqrt(V) * K / parameters.quality + V * K * K) * norm;
            a1 = 2.f * (V * K * K - 1.f) * norm;
            a2 = (1.f - sqrt(V) * K / parameters.quality + V * K * K) * norm;
            b1 = 2.f * (K * K - 1.f) * norm;
            b2 = (1.f - K / parameters.quality + K * K) * norm;
        }
        else
        { // cut
            norm = 1.f / (1.f + sqrt(V) * K / parameters.quality + V * K * K);
            a0 = (1.f + K / parameters.quality + K * K) * norm;
            a1 = 2.f * (K * K - 1.f) * norm;
            a2 = (1 - K / parameters.quality + K * K) * norm;
            b1 = 2.f * (V * K * K - 1.f) * norm;
            b2 = (1.f - sqrt(V) * K / parameters.quality + V * K * K) * norm;
        }
        break;

    case BIQUAD_HIGHSHELF:
        if (parameters.gain >= 0.f)
        { // boost
            norm = 1 / (1.f + K / parameters.quality + K * K);
            a0 = (V + sqrt(V) * K / parameters.quality + K * K) * norm;
            a1 = 2.f * (K * K - V) * norm;
            a2 = (V - sqrt(V) * K / parameters.quality + K * K) * norm;
            b1 = 2.f * (K * K - 1.f) * norm;
            b2 = (1.f - K / parameters.quality + K * K) * norm;
        }
        else
        { // cut
            norm = 1.f / (V + sqrt(V) * K / parameters.quality + K * K);
            a0 = (1.f + K / parameters.quality + K * K) * norm;
            a1 = 2.f * (K * K - 1.f) * norm;
            a2 = (1.f - K / parameters.quality + K * K) * norm;
            b1 = 2.f * (K * K - V) * norm;
            b2 = (V - sqrt(V) * K / parameters.quality + K * K) * norm;
        }
        break;

    case BIQUAD_LOWSHELF_NOQ:
        if (parameters.gain >= 0.f)
        { // boost
            norm = 1.f / (1.f + sqrt(2.f) * K + K * K);
            a0 = (1.f + sqrt(2.f * V) * K + V * K * K) * norm;
            a1 = 2.f * (V * K * K - 1.f) * norm;
            a2 = (1.f - sqrt(2.f * V) * K + V * K * K) * norm;
            b1 = 2.f * (K * K - 1.f) * norm;
            b2 = (1.f - sqrt(2.f) * K + K * K) * norm;
        }
        else
        { // cut
            norm = 1.f / (1.f + sqrt(2.f * V) * K + V * K * K);
            a0 = (1.f + sqrt(2.f) * K + K * K) * norm;
            a1 = 2.f * (K * K - 1.f) * norm;
            a2 = (1.f - sqrt(2.f) * K + K * K) * norm;
            b1 = 2.f * (V * K * K - 1.f) * norm;
            b2 = (1.f - sqrt(2.f * V) * K + V * K * K) * norm;
        }
        break;

    case BIQUAD_HIGHSHELF_NOQ:
        if (parameters.gain >= 0.f)
        { // boost
            norm = 1.f / (1.f + sqrt(2.f) * K + K * K);
            a0 = (V + sqrt(2.f * V) * K + K * K) * norm;
            a1 = 2.f * (K * K - V) * norm;
            a2 = (V - sqrt(2.f * V) * K + K * K) * norm;
            b1 = 2.f * (K * K - 1.f) * norm;
            b2 = (1.f - sqrt(2.f) * K + K * K) * norm;
        }
        else
        { // cut
            norm = 1.f / (V + sqrt(2.f * V) * K + K * K);
            a0 = (1.f + sqrt(2.f) * K + K * K) * norm;
            a1 = 2.f * (K * K - 1.f) * norm;
            a2 = (1.f - sqrt(2.f) * K + K * K) * norm;
            b1 = 2.f * (K * K - V) * norm;
            b2 = (V - sqrt(2.f * V) * K + K * K) * norm;
        }
        break;

    case BIQUAD_ALLPASS:
        norm = 1.f / (1.f + K / parameters.quality + K * K);
        a0 = (1.f - K / parameters.quality + K * K) * norm;
        a1 = 2.f * (K * K - 1.f) * norm;
        a2 = 1.f;
        b1 = a1;
        b2 = a0;
        break;

    case BIQUAD_NONE:
        a0 = 1.f;
        a1 = 0.f;
        a2 = 0.f;
        b1 = 0.f;
        b2 = 0.f;
        break;
    }

    coefficients->a0 = a0;
    coefficients->a1 = a1;
    coefficients->a2 = a2;
    coefficients->b1 = b1;
    coefficients->b2 = b2;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif

#ifndef PX_SATURATOR_H
#define PX_SATURATOR_H

typedef enum
{
    ARCTANGENT,
    TANGENT
} SATURATION_CURVE;

typedef struct
{
    float drive;
    SATURATION_CURVE curve;
} px_saturator;


// ----------------------------------------------------------------------------------------------------


static void px_saturator_initialize(px_saturator* saturator, SATURATION_CURVE curve);
static px_saturator* px_saturator_create(SATURATION_CURVE curve);
static void px_saturator_destroy(px_saturator* saturator);

static void px_saturator_set_drive(px_saturator* saturator, float drive);
static void px_saturator_set_curve(px_saturator* saturator, SATURATION_CURVE curve);
static void px_saturator_mono_process(px_saturator* saturator, float* input);
static void px_saturator_stereo_process(px_saturator* saturator, float* input_left, float* input_right);

static inline float px_saturate_arctangent(float input, float drive);
static inline float px_saturate_tangent(float input, float drive);


// ----------------------------------------------------------------------------------------------------


static void px_saturator_initialize(px_saturator* saturator, SATURATION_CURVE curve)
{
    assert(saturator);
    saturator->drive = 0.f;
    saturator->curve = curve;
}

static px_saturator* px_saturator_create(SATURATION_CURVE curve)
{
    px_saturator* saturator = (px_saturator*)px_malloc(sizeof(px_saturator));
    if (saturator)
    {
        px_saturator_initialize(saturator, curve);
        return saturator;
    }
    else return NULL;
}

static void px_saturator_destroy(px_saturator* saturator)
{
    if (saturator)
	px_free(saturator);
}

static void px_saturator_set_drive(px_saturator* saturator, float drive)
{
    assert(saturator);
    saturator->drive = drive;
}

static void px_saturator_set_curve(px_saturator* saturator, SATURATION_CURVE curve)
{
    assert(saturator);
    saturator->curve = curve;
}

static void px_saturator_mono_process(px_saturator* saturator, float* input)
{
    px_assert(saturator, input);
    switch (saturator->curve)
    {	
	case ARCTANGENT:
		*input = px_saturate_arctangent(*input, saturator->drive);
		break;

	case TANGENT:
		*input = px_saturate_tangent(*input, saturator->drive);	
    		break;
    }
}

static void px_saturator_stereo_process(px_saturator* saturator, float* input_left, float* input_right)
{
    px_assert(saturator, input_left, input_right);
    switch (saturator->curve)
    {	
	case ARCTANGENT:
		*input_left = px_saturate_arctangent(*input_left, saturator->drive);
		*input_right = px_saturate_arctangent(*input_right, saturator->drive);
		break;

	case TANGENT:
		*input_left = px_saturate_tangent(*input_left, saturator->drive);
    		*input_right = px_saturate_tangent(*input_right, saturator->drive);
		break;
    }
}

static inline float px_saturate_arctangent(float input, float drive)
{
    return atan(input * dB2lin(drive));
}

static inline float px_saturate_tangent(float input, float drive)
{
    return tanh(input * dB2lin(drive));
}

#endif
#ifndef PX_CLIP_H
#define PX_CLIP_H


typedef enum
{
    HARD,
    SOFT,	// quintic 
    SMOOTH	// arctangent
} CLIP_TYPE;

typedef struct
{
   CLIP_TYPE type;
} px_clipper;

// -----------------------------------------------------------------------------------------
// api
static px_clipper* px_clipper_create();
static void px_clipper_destroy(px_clipper* clipper);

static void px_clipper_initialize(px_clipper* clipper);
static void px_clipper_set_type(px_clipper* clipper, CLIP_TYPE in_type);

static void px_clipper_mono_process(px_clipper* clipper, float* input);
static void px_clipper_stereo_process(px_clipper* clipper, float* input_left, float* input_right);

// ---------------------------------------------------------------------------------------------
// inline functions
static inline float hard_clip(float input);
static inline float quintic_clip(float input);
static inline float arctangent_clip(float input);
// ---------------------------------------------------------------------------------------------

static px_clipper* px_clipper_create()
{
    px_clipper* clipper = (px_clipper*)malloc(sizeof(px_clipper));
	px_clipper_initialize(clipper);
    return clipper;
}

static void px_clipper_destroy(px_clipper* clipper)
{
    if (clipper)
    	free(clipper);
}

static void px_clipper_initialize(px_clipper* clipper)
{
    assert(clipper);
    clipper->type = HARD;
}

static void px_clipper_set_type(px_clipper* clipper, CLIP_TYPE in_type)
{
    assert(clipper);
    clipper->type = in_type;    
}

static void px_clipper_mono_process(px_clipper* clipper, float* input)
{
	px_assert(clipper, input);
	printf("%d", clipper->type);
	switch (clipper->type)
	{
	case HARD:
	{
		*input = hard_clip(*input);
		break;
	}
	case SOFT:
	{
		*input = quintic_clip(*input);
		break;
	}
	case SMOOTH:
	{
		*input = arctangent_clip(*input);
		break;
	}
	default:
	{
		printf("clipper uninitialized");
		break;
	}
	}
}

static void px_clipper_stereo_process(px_clipper* clipper, float* input_left, float* input_right)
{
	px_assert(clipper, input_left, input_right);
	printf("%d", clipper->type);
	switch (clipper->type)
	{
	case HARD:
	{
		*input_left = hard_clip(*input_left);
		*input_right = hard_clip(*input_right);
		break;
	}
	case SOFT:
	{
		*input_left = quintic_clip(*input_left);
		*input_right = quintic_clip(*input_right);
		break;
	}
	case SMOOTH:
	{
		*input_left = arctangent_clip(*input_left);
		*input_right = arctangent_clip(*input_right);
		break;
	}
	default:
	{
		printf("clipper uninitialized");
		break;
	}
	}
}

static inline float hard_clip(float input)
{
	return sgn(input) * fmin(fabs(input), 1.0f);
}

static inline float quintic_clip(float input)
{
	if (fabs(input) < 1.25f)
		return input - (256.0f / 3125.0f) * powf(input, 5.0f);
	else
		return sgn(input) * 1.0f;
}

static inline float arctangent_clip(float input)
{
	return (2.0f / PI) * atan((1.6f * 0.6f) * input);
}

#endif


#ifndef PX_EQUALIZER_H
#define PX_EQUALIZER_H

#define MAX_BANDS 24

	typedef struct px_mono_equalizer
	{
		px_vector filter_bank;  // type-generic vector filled with px_mono_biquad filters
		float sample_rate;
		int num_bands;
	} px_mono_equalizer;

	typedef struct px_stereo_equalizer
	{
		px_mono_equalizer left;
		px_mono_equalizer right;
	} px_stereo_equalizer;

	typedef struct px_ms_equalizer
	{
		px_mono_equalizer mid;
		px_mono_equalizer side;
	} px_ms_equalizer;

	// ----------------------------------------------------------------------------------------------------

#ifdef PX_USE_GENERIC

#define px_equalizer_process(a,b,...) _Generic((a),		\
	px_mono_equalizer*: px_equalizer_mono_process,		\
	px_stereo_equalizer*: px_equalizer_stereo_process,	\
	px_ms_equalizer*: px_equalizer_ms_process)		\
		(a,b,__VA_ARGS__)					\

#define px_equalizer_initialize(a,b) _Generic((a),		\
	px_mono_equalizer*: px_equalizer_mono_initialize,	\
	px_stereo_equalizer*: px_equalizer_stereo_initialize,	\
	px_ms_equalizer*: px_equalizer_ms_initialize)		\
		(a,b)

#define px_equalizer_add_band(a,b,c,d,e) _Generic((a),		\
	px_mono_equalizer*: px_equalizer_mono_add_band,		\
	px_stereo_equalizer*: px_equalizer_stereo_add_band,	\
	px_ms_equalizer*: px_equalizer_ms_add_band)		\
		(a,b,c,d,e)					\

#define px_equalizer_remove_band(a,b) _Generic((a),		\
	px_mono_equalizer*: px_equalizer_mono_remove_band,	\
	px_stereo_equalizer*: px_equalizer_stereo_remove_band,	\
	px_ms_equalizer*: px_equalizer_ms_remove_band)		\
		(a,b)


#define px_equalizer_set_frequency(a,b,c,...) _Generic((a),		\
	px_mono_equalizer*: px_equalizer_mono_set_frequency,		\
	px_stereo_equalizer*: px_equalizer_stereo_set_frequency,	\
	px_ms_equalizer*: px_equalizer_ms_set_frequency)		\
		(a,b,c,__VA_ARGS__)

#define px_equalizer_set_quality(a,b,c,...) _Generic((a),		\
	px_mono_equalizer*: px_equalizer_mono_set_quality,		\
	px_stereo_equalizer*: px_equalizer_stereo_set_quality,		\
	px_ms_equalizer*: px_equalizer_ms_set_quality)		\
		(a,b,c,__VA_ARGS__)

#define px_equalizer_set_gain(a,b,c,...) _Generic((a),			\
	px_mono_equalizer*: px_equalizer_mono_set_gain,		\
	px_stereo_equalizer*: px_equalizer_stereo_set_gain,		\
	px_ms_equalizer*: px_equalizer_ms_set_quality)		\
		(a,b,c,__VA_ARGS__)

#define px_equalizer_set_type(a,b,c,...) _Generic((a),			\
	px_mono_equalizer*: px_equalizer_mono_set_type,		\
	px_stereo_equalizer*: px_equalizer_stereo_set_type,		\
	px_ms_equalizer*: px_equalizer_ms_set_type)			\
		(a,b,c,__VA_ARGS__)
#endif

// mono
	static void px_equalizer_mono_process(px_mono_equalizer* equalizer, float* input);
	static void px_equalizer_mono_initialize(px_mono_equalizer* equalizer, float sample_rate);
	static void px_equalizer_mono_add_band(px_mono_equalizer* equalizer, float frequency, float quality, float gain, BIQUAD_FILTER_TYPE type);
	static void px_equalizer_mono_remove_band(px_mono_equalizer* equalizer, size_t index);

	static void px_equalizer_mono_set_frequency(px_mono_equalizer* equalizer, size_t index, float in_frequency);
	static void px_equalizer_mono_set_quality(px_mono_equalizer* equalizer, size_t index, float in_quality);
	static void px_equalizer_mono_set_gain(px_mono_equalizer* equalizer, size_t index, float in_gain);
	static void px_equalizer_mono_set_type(px_mono_equalizer* equalizer, size_t index, BIQUAD_FILTER_TYPE in_type);

	// stereo
	static void px_equalizer_stereo_process(px_stereo_equalizer* stereo_equalizer, float* input_left, float* input_right);
	static void px_equalizer_stereo_initialize(px_stereo_equalizer* stereo_equalizer, float sample_rate);
	static void px_equalizer_stereo_add_band(px_stereo_equalizer* stereo_equalizer, float frequency, float quality, float gain, BIQUAD_FILTER_TYPE type);
	static void px_equalizer_stereo_remove_band(px_stereo_equalizer* stereo_equalizer, size_t index);

	static void px_equalizer_stereo_set_frequency(px_stereo_equalizer* stereo_equalizer, size_t index, float in_frequency, CHANNEL_FLAG channel);
	static void px_equalizer_stereo_set_quality(px_stereo_equalizer* stereo_equalizer, size_t index, float in_quality, CHANNEL_FLAG channel);
	static void px_equalizer_stereo_set_gain(px_stereo_equalizer* stereo_equalizer, size_t index, float in_gain, CHANNEL_FLAG channel);
	static void px_equalizer_stereo_set_type(px_stereo_equalizer* stereo_equalizer, size_t index, BIQUAD_FILTER_TYPE in_type, CHANNEL_FLAG channel);

	// mid/side
	static void px_equalizer_ms_process(px_ms_equalizer* ms_equalizer, float* input_left, float* input_right);
	static px_ms_encoded px_equalizer_ms_process_and_return(px_ms_equalizer* ms_equalizer, float input_left, float input_right);
	static void px_equalizer_ms_initialize(px_ms_equalizer* ms_equalizer, float sample_rate);
	static void px_equalizer_ms_add_band(px_ms_equalizer* ms_equalizer, float frequency, float quality, float gain, BIQUAD_FILTER_TYPE type);
	static void px_equalizer_ms_remove_band(px_ms_equalizer* ms_equalizer, size_t index);

	static void px_equalizer_ms_set_frequency(px_ms_equalizer* ms_equalizer, size_t index, float in_frequency, CHANNEL_FLAG channel);
	static void px_equalizer_ms_set_quality(px_ms_equalizer* ms_equalizer, size_t index, float in_quality, CHANNEL_FLAG channel);
	static void px_equalizer_ms_set_gain(px_ms_equalizer* ms_equalizer, size_t index, float in_gain, CHANNEL_FLAG channel);
	static void px_equalizer_ms_set_type(px_ms_equalizer* ms_equalizer, size_t index, BIQUAD_FILTER_TYPE in_type, CHANNEL_FLAG channel);


	// ----------------------------------------------------------------------------------------------------

	static void px_equalizer_mono_process(px_mono_equalizer* equalizer, float* input)
	{
		px_assert(equalizer, input);
		for (int i = 0; i < equalizer->num_bands; ++i)
		{
			px_biquad_process((px_biquad*)px_vector_get(&equalizer->filter_bank, i), input);
		}
	}

	static void px_equalizer_stereo_process(px_stereo_equalizer* stereo_equalizer, float* input_left, float* input_right)
	{
		px_assert(stereo_equalizer, input_left, input_right);
		px_equalizer_mono_process(&stereo_equalizer->left, input_left);
		px_equalizer_mono_process(&stereo_equalizer->right, input_right);

	}

	static void px_equalizer_ms_process(px_ms_equalizer* ms_equalizer, float* input_left, float* input_right)
	{
		px_assert(ms_equalizer, input_left, input_right);
		px_ms_decoded decoded = { 0.f, 0.f };

		decoded.left = *input_left;
		decoded.right = *input_right;

		px_ms_encoded encoded = px_ms_encode(decoded);

		px_equalizer_mono_process(&ms_equalizer->mid, &encoded.mid);
		px_equalizer_mono_process(&ms_equalizer->side, &encoded.side);

		decoded = px_ms_decode(encoded);

		*input_left = decoded.left;
		*input_right = decoded.right;
	}
	
	static px_ms_encoded px_equalizer_ms_process_and_return(px_ms_equalizer* ms_equalizer, float input_left, float input_right)
	{
		assert(ms_equalizer);
		px_ms_decoded decoded = { 0.f, 0.f };

		decoded.left = input_left;
		decoded.right = input_right;

		px_ms_encoded encoded = px_ms_encode(decoded);

		px_equalizer_mono_process(&ms_equalizer->mid, &encoded.mid);
		px_equalizer_mono_process(&ms_equalizer->side, &encoded.side);

		return encoded;
	}

	static void px_equalizer_mono_initialize(px_mono_equalizer* equalizer, float sample_rate)
	{
		assert(equalizer);
		equalizer->sample_rate = sample_rate;
		equalizer->num_bands = 0;

		px_vector_initialize(&equalizer->filter_bank);
	}

	static void px_equalizer_stereo_initialize(px_stereo_equalizer* stereo_equalizer, float sample_rate)
	{
		assert(stereo_equalizer);
		px_equalizer_mono_initialize(&stereo_equalizer->left, sample_rate);
		px_equalizer_mono_initialize(&stereo_equalizer->right, sample_rate);
	}

	static void px_equalizer_ms_initialize(px_ms_equalizer* ms_equalizer, float sample_rate)
	{
		assert(ms_equalizer);
		px_equalizer_mono_initialize(&ms_equalizer->mid, sample_rate);
		px_equalizer_mono_initialize(&ms_equalizer->side, sample_rate);
	}


	static void px_equalizer_mono_add_band(px_mono_equalizer* equalizer, float frequency, float quality, float gain, BIQUAD_FILTER_TYPE type)
	{
		assert(equalizer);
		px_biquad* new_filter = px_biquad_create(equalizer->sample_rate, type);

		px_biquad_set_frequency(new_filter, frequency);
		px_biquad_set_quality(new_filter, quality);
		px_biquad_set_gain(new_filter, gain);

		px_vector_push(&equalizer->filter_bank, new_filter);
		equalizer->num_bands++;
	}

	static void px_equalizer_stereo_add_band(px_stereo_equalizer* stereo_equalizer, float frequency, float quality, float gain, BIQUAD_FILTER_TYPE type)
	{
		assert(stereo_equalizer);
		px_equalizer_mono_add_band(&stereo_equalizer->left, frequency, quality, gain, type);
		px_equalizer_mono_add_band(&stereo_equalizer->right, frequency, quality, gain, type);
	}

	static void px_equalizer_ms_add_band(px_ms_equalizer* ms_equalizer, float frequency, float quality, float gain, BIQUAD_FILTER_TYPE type)
	{
		assert(ms_equalizer);
		px_equalizer_mono_add_band(&ms_equalizer->mid, frequency, quality, gain, type);
		px_equalizer_mono_add_band(&ms_equalizer->side, frequency, quality, gain, type);
	}


	static void px_equalizer_mono_remove_band(px_mono_equalizer* equalizer, size_t index)
	{
		if (index < equalizer->num_bands)
		{
			px_biquad_destroy((px_biquad*)px_vector_get(&equalizer->filter_bank, index));
			px_vector_remove(&equalizer->filter_bank, index);
			equalizer->num_bands--;
		}
	}

	static void px_equalizer_stereo_remove_band(px_stereo_equalizer* stereo_equalizer, size_t index)
	{
		assert(stereo_equalizer);
		px_equalizer_mono_remove_band(&stereo_equalizer->left, index);
		px_equalizer_mono_remove_band(&stereo_equalizer->right, index);
	}


	static void px_equalizer_ms_remove_band(px_ms_equalizer* ms_equalizer, size_t index)
	{
		assert(ms_equalizer);
		px_equalizer_mono_remove_band(&ms_equalizer->mid, index);
		px_equalizer_mono_remove_band(&ms_equalizer->side, index);
	}


	static void px_equalizer_mono_set_frequency(px_mono_equalizer* equalizer, size_t index, float in_frequency)
	{
		assert(equalizer);
		if (index < equalizer->num_bands)
		{
			px_biquad* filter = (px_biquad*)px_vector_get(&equalizer->filter_bank, index);
			px_biquad_set_frequency(filter, in_frequency);
		}
	}

	static void px_equalizer_stereo_set_frequency(px_stereo_equalizer* stereo_equalizer, size_t index, float in_frequency, CHANNEL_FLAG channel)
	{
		assert(stereo_equalizer);
		switch (channel)
		{
		case BOTH:
		{
			px_equalizer_mono_set_frequency(&stereo_equalizer->left, index, in_frequency);
			px_equalizer_mono_set_frequency(&stereo_equalizer->right, index, in_frequency);
		}
		case LEFT:
		{
			px_equalizer_mono_set_frequency(&stereo_equalizer->left, index, in_frequency);
		}
		case RIGHT:
		{
			px_equalizer_mono_set_frequency(&stereo_equalizer->right, index, in_frequency);
		}
		}
	}

	static void px_equalizer_ms_set_frequency(px_ms_equalizer* ms_equalizer, size_t index, float in_frequency, CHANNEL_FLAG channel)
	{
		assert(ms_equalizer);
		switch (channel)
		{
		case BOTH:
		{
			px_equalizer_mono_set_frequency(&ms_equalizer->mid, index, in_frequency);
			px_equalizer_mono_set_frequency(&ms_equalizer->side, index, in_frequency);
		}
		case LEFT:
		{
			px_equalizer_mono_set_frequency(&ms_equalizer->mid, index, in_frequency);
		}
		case RIGHT:
		{
			px_equalizer_mono_set_frequency(&ms_equalizer->side, index, in_frequency);
		}
		}
	}


	static void px_equalizer_mono_set_quality(px_mono_equalizer* equalizer, size_t index, float in_quality)
	{
		assert(equalizer);
		if (index < equalizer->num_bands)
		{
			px_biquad* filter = (px_biquad*)px_vector_get(&equalizer->filter_bank, index);
			px_biquad_set_quality(filter, in_quality);
		}
	}

	static void px_equalizer_stereo_set_quality(px_stereo_equalizer* stereo_equalizer, size_t index, float in_quality, CHANNEL_FLAG channel)
	{
		assert(stereo_equalizer);
		switch (channel)
		{
		case BOTH:
		{
			px_equalizer_mono_set_quality(&stereo_equalizer->left, index, in_quality);
			px_equalizer_mono_set_quality(&stereo_equalizer->right, index, in_quality);
		}
		case LEFT:
		{
			px_equalizer_mono_set_quality(&stereo_equalizer->left, index, in_quality);
		}
		case RIGHT:
		{
			px_equalizer_mono_set_quality(&stereo_equalizer->right, index, in_quality);
		}
		}
	}

	static void px_equalizer_ms_set_quality(px_ms_equalizer* ms_equalizer, size_t index, float in_quality, CHANNEL_FLAG channel)
	{
		assert(ms_equalizer);
		switch (channel)
		{
		case BOTH:
		{
			px_equalizer_mono_set_quality(&ms_equalizer->mid, index, in_quality);
			px_equalizer_mono_set_quality(&ms_equalizer->side, index, in_quality);
		}
		case MID:
		{
			px_equalizer_mono_set_quality(&ms_equalizer->mid, index, in_quality);
		}
		case SIDE:
		{
			px_equalizer_mono_set_quality(&ms_equalizer->side, index, in_quality);
		}
		}
	}


	static void px_equalizer_mono_set_gain(px_mono_equalizer* equalizer, size_t index, float in_gain)
	{
		assert(equalizer);
		if (index < equalizer->num_bands)
		{
			px_biquad* filter = (px_biquad*)px_vector_get(&equalizer->filter_bank, index);
			px_biquad_set_gain(filter, in_gain);
		}
	}

	static void px_equalizer_stereo_set_gain(px_stereo_equalizer* stereo_equalizer, size_t index, float in_gain, CHANNEL_FLAG channel)
	{
		assert(stereo_equalizer);
		switch (channel)
		{
		case BOTH:
		{
			px_equalizer_mono_set_gain(&stereo_equalizer->left, index, in_gain);
			px_equalizer_mono_set_gain(&stereo_equalizer->right, index, in_gain);
		}
		case LEFT:
		{
			px_equalizer_mono_set_gain(&stereo_equalizer->left, index, in_gain);
		}
		case RIGHT:
		{
			px_equalizer_mono_set_gain(&stereo_equalizer->right, index, in_gain);
		}
		}
	}

	static void px_equalizer_ms_set_gain(px_ms_equalizer* ms_equalizer, size_t index, float in_gain, CHANNEL_FLAG channel)
	{
		assert(ms_equalizer);
		switch (channel)
		{
		case BOTH:
		{
			px_equalizer_mono_set_gain(&ms_equalizer->mid, index, in_gain);
			px_equalizer_mono_set_gain(&ms_equalizer->side, index, in_gain);
		}
		case MID:
		{
			px_equalizer_mono_set_gain(&ms_equalizer->mid, index, in_gain);
		}
		case SIDE:
		{
			px_equalizer_mono_set_gain(&ms_equalizer->side, index, in_gain);
		}
		}
	}

	static void px_equalizer_mono_set_type(px_mono_equalizer* equalizer, size_t index, BIQUAD_FILTER_TYPE in_type)
	{
		assert(equalizer);
		if (index < equalizer->num_bands)
		{
			px_biquad* filter = (px_biquad*)px_vector_get(&equalizer->filter_bank, index);
			px_biquad_set_type(filter, in_type);
		}
	}

	static void px_equalizer_stereo_set_type(px_stereo_equalizer* stereo_equalizer, size_t index, BIQUAD_FILTER_TYPE in_type, CHANNEL_FLAG channel)
	{
		assert(stereo_equalizer);
		switch (channel)
		{
		case BOTH:
		{
			px_equalizer_mono_set_type(&stereo_equalizer->left, index, in_type);
			px_equalizer_mono_set_type(&stereo_equalizer->right, index, in_type);
		}
		case LEFT:
		{
			px_equalizer_mono_set_type(&stereo_equalizer->left, index, in_type);
		}
		case RIGHT:
		{
			px_equalizer_mono_set_type(&stereo_equalizer->right, index, in_type);
		}
		}
	}

	static void px_equalizer_ms_set_type(px_ms_equalizer* ms_equalizer, size_t index, BIQUAD_FILTER_TYPE in_type, CHANNEL_FLAG channel)
	{
		assert(ms_equalizer);
		switch (channel)
		{
		case BOTH:
		{
			px_equalizer_mono_set_type(&ms_equalizer->mid, index, in_type);
			px_equalizer_mono_set_type(&ms_equalizer->side, index, in_type);
		}
		case MID:
		{
			px_equalizer_mono_set_type(&ms_equalizer->mid, index, in_type);
		}
		case SIDE:
		{
			px_equalizer_mono_set_type(&ms_equalizer->side, index, in_type);
		}
		}
	}

#endif
#ifndef PX_COMPRESSOR_H
#define PX_COMPRESSOR_H


/*
 
	px_compressor 

		mono, stereo, or mid-side VCA-style compressor with built-in sidechain equalizer and dual_mono process mode
	
	px_compressor_parameters (float)
		
		threshold, ratio, knee width, makeup gain, attack and release
	

	init:
		stack initialize:
			px_mono_compressor compressor;
			px_compressor_mono_initialize(&compressor, sample_rate);
		heap:
			px_mono_compressor* compressor = px_compressor_mono_create(sample_rate);
			// initialize called within create();
			
			px_mono_compressor_destroy(compressor);
			// don't forget to free
	use:

		// pointer to compressor and pointer to buffer float
		px_compressor_mono_process(&compressor, &input) 

		// boolean flag for dual mono processing for stereo and mid-side
		px_compressor_stereo_process(&stereo_compressor, &input_left, &input_right, true) // dual_mono
		px_compressor_ms_process(&ms_compressor, &inpur_left, input_right, false)         // not dual mono
		
		REMEMBER: mid-side encoding done within px_ms_compressor_process() function
*/



typedef struct
{
    float sample_rate;
    float time_constant;
    float coefficient;

} px_envelope_detector;


#define INITIALIZED_PARAMETERS { 0.f, 1.f, 0.f, 0.f, 0.f, 10.f, 100.f }

// public interface
// use functions to set and compressors.parameters.(value) to get 

typedef struct
{
    float threshold;
    float ratio;
    float env;              // over-threshold envelope
    float knee_width;
    float makeup_gain;

    float attack;	    // compressor.attack.time_constant
    float release;	    // compressor.release.time_constant

} px_compressor_parameters;

typedef struct
{
    px_compressor_parameters parameters;
    px_envelope_detector attack;
    px_envelope_detector release;

    px_mono_equalizer sidechain_equalizer;
} px_mono_compressor;

typedef struct 
{
    px_mono_compressor left;
    px_mono_compressor right;
   
    px_compressor_parameters parameters; 
    px_stereo_equalizer sidechain_equalizer;
} px_stereo_compressor;

typedef struct
{
    px_mono_compressor mid;
    px_mono_compressor side;
    
    px_compressor_parameters parameters;
    px_ms_equalizer sidechain_equalizer;    
} px_ms_compressor;
	 
// API functions
// ----------------------------------------------------------------------------------------------------------------------

// explicitly choose mono or stereo to create and destroy

static px_mono_compressor* px_compressor_mono_create(float in_sample_rate);
static void px_compressor_mono_destroy(px_mono_compressor* compressor);

static px_stereo_compressor* px_compressor_stereo_create(float in_sample_rate);
static void px_compressor_stereo_destroy(px_stereo_compressor* compressor);

static px_ms_compressor* px_compressor_ms_create(float in_sample_rate);
static void px_compressor_ms_destroy(px_ms_compressor* compressor);


#ifdef PX_USE_GENERICS

#define px_compressor_process(a,...) _Generic((a),			\
	px_mono_compressor*: px_compressor_mono_process,		\
	px_stereo_compressor*: px_compressor_stereo_process),		\
		(a, __VA_ARGS__)

#define px_compressor_initialize(a,b) _Generic((a),			\
	px_mono_compressor*: px_compressor_mono_initialize,		\
	px_stereo_compressor*: px_compressor_stereo_initialize),	\
		(a,b)

#define px_compressor_set_parameters(a,b) _Generic((a),               	\
	px_mono_compressor*: px_compressor_mono_set_parameters,       	\
	px_stereo_compressor*: px_compressor_stereo_set_parameters),  	\
		(a,b) 		             

#define px_compressor_set_threshold(a,b) _Generic((a),		     	\
	px_mono_compressor*: px_compressor_mono_set_threshold,		\
	px_stereo_compressor*: px_compressor_stereo_set_threshold),	\
		(a,b)

#define px_compressor_set_ratio(a,b) _Generic((a),		     	\
	px_mono_compressor*: px_compressor_mono_set_ratio,		\
	px_stereo_compressor*: px_compressor_stereo_set_ratio),		\
		(a,b)

#define px_compressor_set_knee(a,b) _Generic((a),		     	\
	px_mono_compressor*: px_compressor_mono_set_knee,		\
	px_stereo_compressor*: px_compressor_stereo_set_knee),		\
		(a,b)

#define px_compressor_set_attack(a,b) _Generic((a),		     	\
	px_mono_compressor*: px_compressor_mono_set_attack,		\
	px_stereo_compressor*: px_compressor_stere_set_attack),		\
		(a,b)

#define px_compressor_set_release(a,b) _Generic((a),		     	\
	px_mono_compressor*: px_compressor_mono_set_release,		\
	px_stereo_compressor*: px_compressor_stereo_set_release),	\
		(a,b)

#define px_compressor_set_makeup_gain(a,b) _Generic((a),	     	\
	px_mono_compressor*: px_compressor_mono_set_makeup_gain,	\
	px_stereo_compressor*: px_compressor_stereo_set_makeup_gain),	\
		(a,b)
#endif



// ----------------------------------------------------------------------------------------------------------------------
// mono

static void px_compressor_mono_process(px_mono_compressor* compressor, float* input);
static void px_compressor_mono_initialize(px_mono_compressor* compressor, float in_sample_rate);

static void px_compressor_mono_set_parameters(px_mono_compressor* compressor, px_compressor_parameters in_parameters);
static void px_compressor_mono_set_threshold(px_mono_compressor* compressor, float in_threshold);
static void px_compressor_mono_set_ratio(px_mono_compressor* compressor, float in_ratio);
static void px_compressor_mono_set_knee(px_mono_compressor* compressor, float in_knee_width);
static void px_compressor_mono_set_attack(px_mono_compressor* compressor, float in_attack);
static void px_compressor_mono_set_release(px_mono_compressor* compressor, float in_release);
static void px_compressor_mono_set_makeup_gain(px_mono_compressor* compressor, float in_gain);

static void px_compressor_mono_set_sidechain_frequency(px_mono_compressor* compressor, float in_frequency);
static void px_compressor_mono_set_sidechain_quality(px_mono_compressor* compressor, float in_quality);
static void px_compressor_mono_set_sidechain_gain(px_mono_compressor* compressor, float in_gain);
static void px_compressor_mono_set_sidechain_type(px_mono_compressor* compressor, BIQUAD_FILTER_TYPE in_type);

// stereo

static void px_compressor_stereo_process(px_stereo_compressor* compressor, float* input_left, float* input_right, bool dual_mono);
static void px_compressor_stereo_initialize(px_stereo_compressor* compressor, float in_sample_rate);

static void px_compressor_stereo_set_parameters(px_stereo_compressor* compressor, px_compressor_parameters in_parameters);
static void px_compressor_stereo_set_threshold(px_stereo_compressor* compressor, float in_threshold);
static void px_compressor_stereo_set_ratio(px_stereo_compressor* compressor, float in_ratio);
static void px_compressor_stereo_set_knee(px_stereo_compressor* compressor, float in_knee_width);
static void px_compressor_stereo_set_attack(px_stereo_compressor* compressor, float in_attack);
static void px_compressor_stereo_set_release(px_stereo_compressor* compressor, float in_release);
static void px_compressor_stereo_set_makeup_gain(px_stereo_compressor* compressor, float in_gain);

static void px_compressor_stereo_set_sidechain_frequency(px_stereo_compressor* compressor, float in_frequency, CHANNEL_FLAG channel);
static void px_compressor_stereo_set_sidechain_quality(px_stereo_compressor* compressor, float in_quality, CHANNEL_FLAG channel);
static void px_compressor_stereo_set_sidechain_gain(px_stereo_compressor* compressor, float in_gain, CHANNEL_FLAG channel);
static void px_compressor_stereo_set_sidechain_type(px_stereo_compressor* compressor, BIQUAD_FILTER_TYPE in_type, CHANNEL_FLAG channel);

// ms

static void px_compressor_ms_process(px_ms_compressor* compressor, float* input_left, float* input_right, bool dual_mono);
static void px_compressor_ms_initialize(px_ms_compressor* compressor, float in_sample_rate);

static void px_compressor_ms_set_parameters(px_ms_compressor* compressor, px_compressor_parameters in_parameters);
static void px_compressor_ms_set_threshold(px_ms_compressor* compressor, float in_threshold);
static void px_compressor_ms_set_ratio(px_ms_compressor* compressor, float in_ratio);
static void px_compressor_ms_set_knee(px_ms_compressor* compressor, float in_knee_width);
static void px_compressor_ms_set_attack(px_ms_compressor* compressor, float in_attack);
static void px_compressor_ms_set_release(px_ms_compressor* compressor, float in_release);
static void px_compressor_ms_set_makeup_gain(px_ms_compressor* compressor, float in_gain);

static void px_compressor_ms_set_sidechain_frequency(px_ms_compressor* compressor, float in_frequency, CHANNEL_FLAG channel);
static void px_compressor_ms_set_sidechain_quality(px_ms_compressor* compressor, float in_quality, CHANNEL_FLAG channel);
static void px_compressor_ms_set_sidechain_gain(px_ms_compressor* compressor, float in_gain, CHANNEL_FLAG channel);
static void px_compressor_ms_set_sidechain_type(px_ms_compressor* compressor, BIQUAD_FILTER_TYPE in_type, CHANNEL_FLAG channel);
// ----------------------------------------------------------------------------------------------------------------------

// inline functions 
// ----------------------------------------------------------------------------------------------------------------------

static inline void px_envelope_detector_calculate_coefficient(px_envelope_detector* envelope);
static inline void px_envelope_detector_run(const px_envelope_detector* envelope, float in, float* state);

static inline void px_compressor_calculate_envelope(const px_mono_compressor* compressor, float in, float* state);
static inline float px_compressor_calculate_knee(const px_mono_compressor* compressor, float overdB); // takes in dB value returns linear (.f)
static inline float px_compressor_compress(px_mono_compressor* compressor, float input, float sidechain);
	
// --------------------------------------------------------------------------------------------------------

static px_mono_compressor* px_compressor_mono_create(float in_sample_rate)
{
    px_mono_compressor* compressor = (px_mono_compressor*)px_malloc(sizeof(px_mono_compressor));
    if (compressor)
	px_compressor_mono_initialize(compressor, in_sample_rate);
	return compressor;

}


static px_stereo_compressor* px_compressor_stereo_create(float in_sample_rate)
{
    px_stereo_compressor* compressor = (px_stereo_compressor*)px_malloc(sizeof(px_stereo_compressor));
    if (compressor)
	px_compressor_stereo_initialize(compressor, in_sample_rate);
	return compressor;

}
static px_ms_compressor* px_compressor_ms_create(float in_sample_rate)
{
    px_ms_compressor* compressor = (px_ms_compressor*)px_malloc(sizeof(px_ms_compressor));
    if (compressor)
	px_compressor_ms_initialize(compressor, in_sample_rate);
        return compressor;
}

static void px_compressor_mono_destroy(px_mono_compressor* compressor)
{
    if (compressor)
	px_free(compressor);
}

static void px_compressor_stereo_destroy(px_stereo_compressor* compressor)
{
    if (compressor)
	px_free(compressor);
}

static void px_compressor_ms_destroy(px_ms_compressor* compressor)
{
    if (compressor)
	px_free(compressor);
}

static void px_compressor_mono_process(px_mono_compressor* compressor, float* input)
{
    // check the caclulate_envelope function
    px_assert(compressor, input);
    
    //sidechain eq
    float sidechain = *input;
    px_equalizer_mono_process(&compressor->sidechain_equalizer, &sidechain);
    *input = px_compressor_compress(compressor, *input, sidechain);
}

static void px_compressor_stereo_process(px_stereo_compressor* compressor, float* input_left, float* input_right, bool dual_mono)
{
    px_assert(compressor, input_left, input_right);

    //sidechain eq
    float sidechain_left = *input_left;
    float sidechain_right = *input_right;

    px_equalizer_stereo_process(&compressor->sidechain_equalizer, &sidechain_left, &sidechain_right);

    float input_absolute_left = fabsf(sidechain_left);
    float input_absolute_right = fabsf(sidechain_right); /* put here: rms smoothing */

    //mono sum
    float input_link = fabsf(fmaxf(input_absolute_left, input_absolute_right));
   
    if (dual_mono)
    {
	*input_left = px_compressor_compress(&compressor->left, *input_left, input_absolute_left);
	*input_right = px_compressor_compress(&compressor->right, *input_right, input_absolute_right);
    }
    else
    {
    	*input_left = px_compressor_compress(&compressor->left, *input_left, input_link);
    	*input_right = px_compressor_compress(&compressor->right, *input_right, input_link);
    }
}

static void px_compressor_ms_process(px_ms_compressor* compressor, float* input_left, float* input_right, bool dual_mono)
{
    px_assert(compressor, input_left, input_right);
    
    float sidechain_left = *input_left;
    float sidechain_right = *input_right;

    px_ms_encoded encoded_sidechain = px_equalizer_ms_process_and_return(&compressor->sidechain_equalizer, sidechain_left, sidechain_right);
   
    float absolute_mid = fabsf(encoded_sidechain.mid);
    float absolute_side = fabsf(encoded_sidechain.side);

    float link = fabsf(fmaxf(absolute_mid, absolute_side));

    px_ms_decoded decoded = { 0.f, 0.f };

    decoded.left = *input_left;
    decoded.right = *input_right;

    px_ms_encoded encoded = px_ms_encode(decoded);

    if (dual_mono)
    {
	encoded.mid = px_compressor_compress(&compressor->mid, encoded.mid, absolute_mid);
	encoded.side = px_compressor_compress(&compressor->side, encoded.side, absolute_side);
    }
    else
    {
    	encoded.mid = px_compressor_compress(&compressor->mid, encoded.mid, link);
    	encoded.side = px_compressor_compress(&compressor->side, encoded.side, link);
    }

    decoded = px_ms_decode(encoded);

    *input_left = decoded.left;
    *input_right = decoded.right;

}

static void px_compressor_mono_initialize(px_mono_compressor* compressor, float in_sample_rate)
{
    assert(compressor);

    px_mono_equalizer equalizer;
    px_equalizer_mono_initialize(&equalizer, in_sample_rate);
    px_equalizer_mono_add_band(&equalizer, 0.f, 1.f, 0.f, BIQUAD_HIGHPASS);
    compressor->sidechain_equalizer = equalizer;

    px_compressor_parameters new_parameters = INITIALIZED_PARAMETERS;
    compressor->parameters = new_parameters;

    compressor->attack.sample_rate = in_sample_rate;
    compressor->release.sample_rate = in_sample_rate;

    compressor->attack.time_constant = 10.f;
    compressor->release.time_constant = 100.f;

    px_envelope_detector_calculate_coefficient(&compressor->attack);
    px_envelope_detector_calculate_coefficient(&compressor->release);
}

static void px_compressor_stereo_initialize(px_stereo_compressor* compressor, float in_sample_rate)
{
    assert(compressor);

    px_stereo_equalizer equalizer;
    px_equalizer_stereo_initialize(&equalizer, in_sample_rate);
    px_equalizer_stereo_add_band(&equalizer, 0.f, 1.f, 0.f, BIQUAD_HIGHPASS);

    compressor->sidechain_equalizer = equalizer;

    px_compressor_parameters new_parameters = INITIALIZED_PARAMETERS; 
    compressor->parameters = new_parameters;


    px_compressor_mono_initialize(&compressor->left, in_sample_rate);
    px_compressor_mono_initialize(&compressor->right, in_sample_rate);

}

static void px_compressor_ms_initialize(px_ms_compressor* compressor, float in_sample_rate)
{
    assert(compressor);

    px_ms_equalizer equalizer;
    px_equalizer_ms_initialize(&equalizer, in_sample_rate);
    px_equalizer_ms_add_band(&equalizer, 0.f, 1.f, 0.f, BIQUAD_HIGHPASS);
    
    compressor->sidechain_equalizer = equalizer;

    px_compressor_parameters new_parameters = INITIALIZED_PARAMETERS;
    compressor->parameters = new_parameters;

    px_compressor_mono_initialize(&compressor->mid, in_sample_rate);
    px_compressor_mono_initialize(&compressor->side, in_sample_rate);

}

static void px_compressor_mono_set_parameters(px_mono_compressor* compressor, px_compressor_parameters in_parameters)
{
    assert(compressor);
    compressor->parameters = in_parameters;
}

static void px_compressor_stereo_set_parameters(px_stereo_compressor* compressor, px_compressor_parameters in_parameters)
{
    assert(compressor);
    compressor->parameters = in_parameters;
    px_compressor_mono_set_parameters(&compressor->left, in_parameters);
    px_compressor_mono_set_parameters(&compressor->right, in_parameters);
}

static void px_compressor_ms_set_parameters(px_ms_compressor* compressor, px_compressor_parameters in_parameters)
{
    assert(compressor);
    compressor->parameters = in_parameters;
    px_compressor_mono_set_parameters(&compressor->mid, in_parameters);
    px_compressor_mono_set_parameters(&compressor->side, in_parameters);
}

static void px_compressor_mono_set_threshold(px_mono_compressor* compressor, float in_threshold)
{
    assert(compressor);
    compressor->parameters.threshold = in_threshold;
}

static void px_compressor_stereo_set_threshold(px_stereo_compressor* compressor, float in_threshold)
{
    assert(compressor);
    compressor->parameters.threshold = in_threshold;
    px_compressor_mono_set_threshold(&compressor->left, in_threshold);
    px_compressor_mono_set_threshold(&compressor->right, in_threshold);
}

static void px_compressor_ms_set_threshold(px_ms_compressor* compressor, float in_threshold)
{
    assert(compressor);
    compressor->parameters.threshold = in_threshold;
    px_compressor_mono_set_threshold(&compressor->mid, in_threshold);
    px_compressor_mono_set_threshold(&compressor->side, in_threshold);
}

static void px_compressor_mono_set_ratio(px_mono_compressor* compressor, float in_ratio)
{
    assert(compressor);
    compressor->parameters.ratio = in_ratio;
}

static void px_compressor_stereo_set_ratio(px_stereo_compressor* compressor, float in_ratio)
{
    assert(compressor);
    compressor->parameters.ratio = in_ratio;
    px_compressor_mono_set_ratio(&compressor->left, in_ratio);
    px_compressor_mono_set_ratio(&compressor->right, in_ratio);
}

static void px_compressor_ms_set_ratio(px_ms_compressor* compressor, float in_ratio)
{
    assert(compressor);
    compressor->parameters.ratio = in_ratio;
    px_compressor_mono_set_ratio(&compressor->mid, in_ratio);
    px_compressor_mono_set_ratio(&compressor->side, in_ratio);
}

static void px_compressor_mono_set_knee(px_mono_compressor* compressor, float in_knee_width)
{
    assert(compressor);
    compressor->parameters.knee_width = in_knee_width;
}

static void px_compressor_stereo_set_knee(px_stereo_compressor* compressor, float in_knee_width)
{
    assert(compressor);
    compressor->parameters.knee_width = in_knee_width;
    px_compressor_mono_set_knee(&compressor->left, in_knee_width);
    px_compressor_mono_set_knee(&compressor->right, in_knee_width);
}

static void px_compressor_ms_set_knee(px_ms_compressor* compressor, float in_knee_width)
{
    assert(compressor);
    compressor->parameters.knee_width = in_knee_width;
    px_compressor_mono_set_knee(&compressor->mid, in_knee_width);
    px_compressor_mono_set_knee(&compressor->side, in_knee_width);
}

static void px_compressor_mono_set_attack(px_mono_compressor* compressor, float in_attack)
{
    assert(compressor);
    compressor->parameters.attack = in_attack;
    compressor->attack.time_constant = in_attack;
    px_envelope_detector_calculate_coefficient(&compressor->attack);
}

static void px_compressor_stereo_set_attack(px_stereo_compressor* compressor, float in_attack)
{
    assert(compressor);
    compressor->parameters.attack = in_attack;
    px_compressor_mono_set_attack(&compressor->left, in_attack);
    px_compressor_mono_set_attack(&compressor->right, in_attack);
}

static void px_compressor_ms_set_attack(px_ms_compressor* compressor, float in_attack)
{
    assert(compressor);
    compressor->parameters.attack = in_attack;
    px_compressor_mono_set_attack(&compressor->mid, in_attack);
    px_compressor_mono_set_attack(&compressor->side, in_attack);
}

static void px_compressor_mono_set_release(px_mono_compressor* compressor, float in_release)
{
    assert(compressor);
    compressor->parameters.release = in_release;
    compressor->release.time_constant = in_release;
    px_envelope_detector_calculate_coefficient(&compressor->release);
}

static void px_compressor_stereo_set_release(px_stereo_compressor* compressor, float in_release)
{
    assert(compressor);
    compressor->parameters.release = in_release;
    px_compressor_mono_set_release(&compressor->left, in_release);
    px_compressor_mono_set_release(&compressor->right, in_release);
}

static void px_compressor_ms_set_release(px_ms_compressor* compressor, float in_release)
{
    assert(compressor);
    compressor->parameters.release = in_release;
    px_compressor_mono_set_release(&compressor->mid, in_release);
    px_compressor_mono_set_release(&compressor->side, in_release);
}

static void px_compressor_mono_set_makeup_gain(px_mono_compressor* compressor, float in_gain)
{
    assert(compressor);
    compressor->parameters.makeup_gain = in_gain;
}

static void px_compressor_stereo_set_makeup_gain(px_stereo_compressor* compressor, float in_gain)
{
    assert(compressor);
    compressor->parameters.makeup_gain = in_gain;
    px_compressor_mono_set_makeup_gain(&compressor->left, in_gain);
    px_compressor_mono_set_makeup_gain(&compressor->right, in_gain);
}

static void px_compressor_ms_set_makeup_gain(px_ms_compressor* compressor, float in_gain)
{
    assert(compressor);
    compressor->parameters.makeup_gain = in_gain;
    px_compressor_mono_set_makeup_gain(&compressor->mid, in_gain);
    px_compressor_mono_set_makeup_gain(&compressor->side, in_gain);
}

// sidechain 

static void px_compressor_mono_set_sidechain_frequency(px_mono_compressor* compressor, float in_frequency)
{
    assert(compressor);
    px_equalizer_mono_set_frequency(&compressor->sidechain_equalizer, 0, in_frequency);
}

static void px_compressor_stereo_set_sidechain_frequency(px_stereo_compressor* compressor, float in_frequency, CHANNEL_FLAG channel)
{
    assert(compressor);
    px_equalizer_stereo_set_frequency(&compressor->sidechain_equalizer, 0, in_frequency, channel);
}

static void px_compressor_ms_set_sidechain_frequency(px_ms_compressor* compressor, float in_frequency, CHANNEL_FLAG channel)
{
    assert(compressor);
    px_equalizer_ms_set_frequency(&compressor->sidechain_equalizer, 0, in_frequency, channel);
}

static void px_compressor_mono_set_sidechain_quality(px_mono_compressor* compressor, float in_quality)
{
    assert(compressor);
    px_equalizer_mono_set_quality(&compressor->sidechain_equalizer, 0, in_quality);
}

static void px_compressor_stereo_set_sidechain_quality(px_stereo_compressor* compressor, float in_quality, CHANNEL_FLAG channel)
{
    assert(compressor);
    px_equalizer_stereo_set_quality(&compressor->sidechain_equalizer, 0, in_quality, channel);
}

static void px_compressor_ms_set_sidechain_quality(px_ms_compressor* compressor, float in_quality, CHANNEL_FLAG channel)
{
    assert(compressor);
    px_equalizer_ms_set_quality(&compressor->sidechain_equalizer, 0, in_quality, channel);
}

static void px_compressor_mono_set_sidechain_gain(px_mono_compressor* compressor, float in_gain)
{
    assert(compressor);
    px_equalizer_mono_set_gain(&compressor->sidechain_equalizer, 0, in_gain);
}

static void px_compressor_stereo_set_sidechain_gain(px_stereo_compressor* compressor, float in_gain, CHANNEL_FLAG channel)
{
    assert(compressor);
    px_equalizer_stereo_set_gain(&compressor->sidechain_equalizer, 0, in_gain, channel);
}

static void px_compressor_ms_set_sidechain_gain(px_ms_compressor* compressor, float in_gain, CHANNEL_FLAG channel)
{
    assert(compressor);
    px_equalizer_ms_set_gain(&compressor->sidechain_equalizer, 0, in_gain, channel);
}

static void px_compressor_mono_set_sidechain_type(px_mono_compressor* compressor, BIQUAD_FILTER_TYPE in_type)
{
    assert(compressor);
    px_equalizer_mono_set_type(&compressor->sidechain_equalizer, 0, in_type);
}

static void px_compressor_stereo_set_sidechain_type(px_stereo_compressor* compressor, BIQUAD_FILTER_TYPE in_type, CHANNEL_FLAG channel)
{
    assert(compressor);
    px_equalizer_stereo_set_type(&compressor->sidechain_equalizer, 0, in_type, channel);
}

static void px_compressor_ms_set_sidechain_type(px_ms_compressor* compressor, BIQUAD_FILTER_TYPE in_type, CHANNEL_FLAG channel)
{
    assert(compressor);
    px_equalizer_ms_set_type(&compressor->sidechain_equalizer, 0, in_type, channel);
}

// ----------------------------------------------------------------------------------------------------------------------

static inline void px_envelope_detector_calculate_coefficient(px_envelope_detector* envelope)
{
    envelope->coefficient = exp(-1000.f / (envelope->time_constant * envelope->sample_rate));
}

static inline void px_envelope_detector_run(const px_envelope_detector* envelope, float in, float* state)
{
    *state = in + envelope->coefficient * (*state - in);
}


static inline void px_compressor_calculate_envelope(const px_mono_compressor* compressor, float in, float* state)
{
    if (in > *state)
    {
        px_envelope_detector_run(&compressor->attack, in, state);
    }
    else
    {
        px_envelope_detector_run(&compressor->release, in, state);
    }
}

// takes in dB value returns linear (.f)
static inline float px_compressor_calculate_knee(const px_mono_compressor* compressor, float overdB)
{
    float knee_start = compressor->parameters.threshold - compressor->parameters.knee_width / 2.0;
    float knee_end = compressor->parameters.threshold + compressor->parameters.knee_width / 2.0;

    
    float gain = 1.0;
        // no compression
    if (overdB <= knee_start) {
        gain = 1.0;
    }
    else if (overdB >= knee_end) {
        // Full compression above the knee
        float reduced_level = overdB / compressor->parameters.ratio;
        gain = dB2lin(-(overdB - reduced_level));
    }
    else {
        // soft knee
        // Within the knee, interpolate the gain reduction
        float blend = (overdB - knee_start) / compressor->parameters.knee_width; 
        float uncompressed_gain = 1.0;
        float reduced_level = overdB / compressor->parameters.ratio;
        float compressed_gain = dB2lin(-(overdB - reduced_level));
        gain = uncompressed_gain + blend * (compressed_gain - uncompressed_gain);
    }

    return gain;
}


static inline float px_compressor_compress(px_mono_compressor* compressor, float input, float sidechain)
{
    // if hit, check the caclulate_envelope function
   // assert(!isnan(compressor->parameters.env));

    sidechain += DC_OFFSET;   // avoid log( 0 )
    float keydB = lin2dB(sidechain); 

    //threshold
    float overdB = keydB - compressor->parameters.threshold;
    if (overdB < 0.f)
        overdB = 0.f;
    
    //attack/release
    overdB += DC_OFFSET;  // avoid denormal
    px_compressor_calculate_envelope(compressor, overdB, &compressor->parameters.env);
    overdB = compressor->parameters.env - DC_OFFSET;

    //transfer function
    float gain_reduction;
    if (compressor->parameters.knee_width > 0.f)
    {
        gain_reduction = px_compressor_calculate_knee(compressor, overdB); //return linear
    }
    else
    {
        gain_reduction = dB2lin(-overdB);
    }

    float output = input * gain_reduction;

    //makeup gain
    float makeup = dB2lin(compressor->parameters.makeup_gain);
    output *= makeup;
    return output;

}	

#endif
