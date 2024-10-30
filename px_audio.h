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
#ifndef PX_DELAY_H
#define PX_DELAY_H
#define PX_FLOAT_BUFFER 1

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
    px_delay_line left;
    px_delay_line right;
    px_delay_parameters parameters;
} px_stereo_delay;


static void px_delay_mono_initialize(px_delay_line* delay, float sample_rate, float max_time)
{
   assert(delay);

   delay_time time = { 1.f, 0.f, 1 };
   px_delay_parameters parameters = {  sample_rate, 0.5f, time, max_time, 0.5f };
   delay->parameters = parameters;

   int max_samples = sample_rate * max_time;
   px_circular_initialize(&delay->buffer, max_samples);
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

static void px_delay_mono_set_feedback(px_delay_line* delay, float feedback)
{
   assert(delay);
   assert(feedback < 1.01f);

   delay->parameters.feedback = feedback;
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

	typedef enum
	{
		BOTH = 0,
		LEFT,
		RIGHT,
		MID,
		SIDE
	} CHANNEL_FLAG;

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
