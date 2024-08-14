#include "px_biquad.h"
#include "px_vector.h"

#define MAX_BANDS 8


typedef struct 
{
    px_mono_biquad filter_bank[MAX_BANDS];
    float sample_rate;
    int num_bands;
} px_mono_equalizer;

typedef struct 
{
   px_vector filter_bank;
   float sample_rate;
   int num_bands;
} px_stereo_equalizer;

static void px_equalizer_mono_process(px_mono_equalizer* equalizer, float* input)
{
     assert(equalizer);
     for (int i = 0; i < equalizer->num_bands; ++i)
     {
     	px_biquad_mono_process(&equalizer->filter_bank[i], input);
     }
}

static void px_equalizer_stereo_process(px_stereo_equalizer* stereo_equalizer, float* input_left, float* input_right)
{
     assert(stereo_equalizer);
     for (int i = 0; i < stereo_equalizer->num_bands; ++i)
     {
     	px_biquad_stereo_process(px_vector_get(&stereo_equalizer->filter_bank, i), input_left, input_right);
     }     
}

static void px_equalizer_mono_initialize(px_mono_equalizer* equalizer, float sample_rate)
{
	assert(equalizer);
	equalizer->sample_rate = sample_rate;
	equalizer->num_bands = 0;
}

static void px_equalizer_stereo_initialize(px_stereo_equalizer* stereo_equalizer, float sample_rate)
{
    assert(stereo_equalizer);
    stereo_equalizer->sample_rate = sample_rate;
    stereo_equalizer->num_bands = 0;
}

static void px_equalizer_mono_add_band(px_mono_equalizer* equalizer, float frequency, float quality, float gain, BIQUAD_FILTER_TYPE type)
{
    assert(equalizer);
    if (equalizer->num_bands + 1 == MAX_BANDS)
    {
	printf("max bands reached");
	return;
    }
    px_mono_biquad new_filter;
    px_biquad_mono_initialize(&new_filter, equalizer->sample_rate, type);
    px_biquad_mono_set_frequency(&new_filter, frequency);
    px_biquad_mono_set_quality(&new_filter, quality);
    px_biquad_mono_set_gain(&new_filter, gain);

    equalizer->filter_bank[equalizer->num_bands] = new_filter;
    equalizer->num_bands++;
}

static void px_equalizer_stereo_add_band(px_stereo_equalizer* stereo_equalizer, float frequency, float quality, float gain, BIQUAD_FILTER_TYPE type)
{
    assert(stereo_equalizer);
    px_stereo_biquad new_filter;

    px_biquad_stereo_initialize(&new_filter, stereo_equalizer->sample_rate, type);
    px_biquad_stereo_set_frequency(&new_filter, frequency);
    px_biquad_stereo_set_quality(&new_filter, quality);
    px_biquad_stereo_set_gain(&new_filter, gain);

    px_vector_push(&stereo_equalizer->filter_bank, &new_filter);
    stereo_equalizer->num_bands++;
}

