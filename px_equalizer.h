#include "px_biquad.h"
#include "px_vector.h"

#define MAX_BANDS 24


typedef struct 
{
    px_vector filter_bank;
    float sample_rate;
    int num_bands;
} px_mono_equalizer;

typedef struct 
{
   px_vector filter_bank;
   float sample_rate;
   int num_bands;
} px_stereo_equalizer;


// ----------------------------------------------------------------------------------------------------

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

static void px_equalizer_stereo_set_frequency(px_stereo_equalizer* stereo_equalizer, size_t index, float in_frequency);
static void px_equalizer_stereo_set_quality(px_stereo_equalizer* stereo_equalizer, size_t index, float in_quality);
static void px_equalizer_stereo_set_gain(px_stereo_equalizer* stereo_equalizer, size_t index, float in_gain);
static void px_equalizer_stereo_set_type(px_stereo_equalizer* stereo_equalizer, size_t index, BIQUAD_FILTER_TYPE in_type);


// ----------------------------------------------------------------------------------------------------

static void px_equalizer_mono_process(px_mono_equalizer* equalizer, float* input)
{
     assert(equalizer);
     for (int i = 0; i < equalizer->num_bands; ++i)
     {
     	px_biquad_mono_process((px_mono_biquad*)px_vector_get(&equalizer->filter_bank, i), input);
     }
}

static void px_equalizer_stereo_process(px_stereo_equalizer* stereo_equalizer, float* input_left, float* input_right)
{
     assert(stereo_equalizer);
     for (int i = 0; i < stereo_equalizer->num_bands; ++i)
     {
     	px_biquad_stereo_process((px_stereo_biquad*)px_vector_get(&stereo_equalizer->filter_bank, i), input_left, input_right);
     }     
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
    stereo_equalizer->sample_rate = sample_rate;
    stereo_equalizer->num_bands = 0;

    px_vector_initialize(&stereo_equalizer->filter_bank);
}

static void px_equalizer_mono_add_band(px_mono_equalizer* equalizer, float frequency, float quality, float gain, BIQUAD_FILTER_TYPE type)
{
    assert(equalizer);
    px_mono_biquad* new_filter = px_biquad_mono_create(equalizer->sample_rate, type);

    px_biquad_mono_set_frequency(new_filter, frequency);
    px_biquad_mono_set_quality(new_filter, quality);
    px_biquad_mono_set_gain(new_filter, gain);

    px_vector_push(&equalizer->filter_bank, new_filter);
    equalizer->num_bands++;
}

static void px_equalizer_stereo_add_band(px_stereo_equalizer* stereo_equalizer, float frequency, float quality, float gain, BIQUAD_FILTER_TYPE type)
{
    assert(stereo_equalizer);
    px_stereo_biquad* new_filter = px_biquad_stereo_create(stereo_equalizer->sample_rate, type);

    px_biquad_stereo_set_frequency(new_filter, frequency);
    px_biquad_stereo_set_quality(new_filter, quality);
    px_biquad_stereo_set_gain(new_filter, gain);

    px_vector_push(&stereo_equalizer->filter_bank, new_filter);
    stereo_equalizer->num_bands++;
}

static void px_equalizer_mono_remove_band(px_mono_equalizer* equalizer, size_t index)
{
    if (index < equalizer->num_bands)
    {
	px_biquad_mono_destroy((px_mono_biquad*)px_vector_get(&equalizer->filter_bank, index));    
	px_vector_remove(&equalizer->filter_bank, index);
	equalizer->num_bands--;
    }
}

static void px_equalizer_stereo_remove_band(px_stereo_equalizer* stereo_equalizer, size_t index)
{
    if (index < stereo_equalizer->num_bands)
    {
	px_biquad_stereo_destroy((px_stereo_biquad*)px_vector_get(&stereo_equalizer->filter_bank, index));
	px_vector_remove(&stereo_equalizer->filter_bank, index);
	stereo_equalizer->num_bands--;
    }
}


static void px_equalizer_mono_set_frequency(px_mono_equalizer* equalizer, size_t index, float in_frequency)
{
    assert(equalizer);
    if (index < equalizer->num_bands)
    {
	px_mono_biquad* filter = (px_mono_biquad*)px_vector_get(&equalizer->filter_bank, index);
	px_biquad_mono_set_frequency(filter, in_frequency);
    }
}

static void px_equalizer_stereo_set_frequency(px_stereo_equalizer* stereo_equalizer, size_t index, float in_frequency)
{
    assert(stereo_equalizer);
    if (index < stereo_equalizer->num_bands)
    {
	px_stereo_biquad* filter = (px_stereo_biquad*)px_vector_get(&stereo_equalizer->filter_bank, index);
	px_biquad_stereo_set_frequency(filter, in_frequency);
    }
}

static void px_equalizer_mono_set_quality(px_mono_equalizer* equalizer, size_t index, float in_quality)
{
    assert(equalizer);
    if (index < equalizer->num_bands)
    {
	px_mono_biquad* filter = (px_mono_biquad*)px_vector_get(&equalizer->filter_bank, index);
	px_biquad_mono_set_quality(filter, in_quality);
    }
}

static void px_equalizer_stereo_set_quality(px_stereo_equalizer* stereo_equalizer, size_t index, float in_quality)
{
    assert(stereo_equalizer);
    if (index < stereo_equalizer->num_bands)
    {
	px_stereo_biquad* filter = (px_stereo_biquad*)px_vector_get(&stereo_equalizer->filter_bank, index);
	px_biquad_stereo_set_quality(filter, in_quality);
    }
}

static void px_equalizer_mono_set_gain(px_mono_equalizer* equalizer, size_t index, float in_gain)
{
    assert(equalizer);
    if (index < equalizer->num_bands)
    {
	px_mono_biquad* filter = (px_mono_biquad*)px_vector_get(&equalizer->filter_bank, index);
	px_biquad_mono_set_gain(filter, in_gain);
    }
}

static void px_equalizer_stereo_set_gain(px_stereo_equalizer* stereo_equalizer, size_t index, float in_gain)
{
    assert(stereo_equalizer);
    if (index < stereo_equalizer->num_bands)
    {
	px_stereo_biquad* filter = (px_stereo_biquad*)px_vector_get(&stereo_equalizer->filter_bank, index);
	px_biquad_stereo_set_gain(filter, in_gain);
    }
}

static void px_equalizer_mono_set_type(px_mono_equalizer* equalizer, size_t index, BIQUAD_FILTER_TYPE in_type)
{
    assert(equalizer);
    if (index < equalizer->num_bands)
    {
	px_mono_biquad* filter = (px_mono_biquad*)px_vector_get(&equalizer->filter_bank, index);
	px_biquad_mono_set_type(filter, in_type);
    }
}

static void px_equalizer_stereo_set_type(px_stereo_equalizer* stereo_equalizer, size_t index, BIQUAD_FILTER_TYPE in_type)
{
    assert(stereo_equalizer);
    if (index < stereo_equalizer->num_bands)
    {
	px_stereo_biquad* filter = (px_stereo_biquad*)px_vector_get(&stereo_equalizer->filter_bank, index);
	px_biquad_stereo_set_type(filter, in_type);
    }

}

