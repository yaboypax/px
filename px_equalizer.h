#include "px_biquad.h"

#define MAX_BANDS 8


typedef struct {
    px_mono_biquad filterBank[MAX_BANDS];
    float sampleRate;
    int numBands;
} px_mono_equalizer;

typedef struct {
   px_mono_equalizer left;
   px_mono_equalizer right;
} px_stereo_equalizer;

static void px_equalizer_mono_process(px_mono_equalizer* equalizer, float* input)
{
     assert(equalizer);
     for (int i = 0; i < equalizer->numBands; ++i)
     {
     	px_biquad_mono_process(&equalizer->filterBank[i], input);
     }
}

static void px_equalizer_stereo_process(px_stereo_equalizer* stereoEqualizer, float* inputLeft, float* inputRight)
{
    assert(stereoEqualizer);
    px_equalizer_mono_process(&stereoEqualizer->left, inputLeft);
    px_equalizer_mono_process(&stereoEqualizer->right, inputRight);
}

static void px_equalizer_mono_initialize(px_mono_equalizer* equalizer, float sampleRate)
{
	assert(equalizer);
	equalizer->sampleRate = sampleRate;
	equalizer->numBands = 0;
}

static void px_equalizer_stereo_initialize(px_stereo_equalizer* stereoEqualizer, float sampleRate)
{
    assert(stereoEqualizer);
    px_equalizer_mono_initialize(&stereoEqualizer->left, sampleRate);
    px_equalizer_mono_initialize(&stereoEqualizer->right, sampleRate);
}

static void px_equalizer_mono_add_band(px_mono_equalizer* equalizer, float frequency, float quality, float gain, BiquadFilterType type)
{
    assert(equalizer);
    if (equalizer->numBands + 1 == MAX_BANDS)
    {
	printf("max bands reached");
	return;
    }
    px_mono_biquad newFilter;
    px_biquad_mono_initialize(&newFilter, equalizer->sampleRate, type);
    px_biquad_mono_set_frequency(&newFilter, frequency);
    px_biquad_mono_set_quality(&newFilter, quality);
    px_biquad_mono_set_gain(&newFilter, gain);

    equalizer->filterBank[equalizer->numBands] = newFilter;
    equalizer->numBands++;
}

static void px_stereo_equalizer_add_band(px_stereo_equalizer* stereoEqualizer, float frequency, float quality, float gain, BiquadFilterType type)
{
    assert(stereoEqualizer);
    px_equalizer_mono_add_band(&stereoEqualizer->left, frequency, quality, gain, type);
    px_equalizer_mono_add_band(&stereoEqualizer->right, frequency, quality, gain, type);
}

