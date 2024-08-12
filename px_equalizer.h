#include "px_biquad.h"

#define MAX_BANDS 8


typedef struct {

    px_mono_biquad filterBank[MAX_BANDS];
    int numBands
} px_mono_equalizer;

typedef struct {
    px_stereo_biquad stereoFilterBank[MAX_BANDS];
    int numBands;
} px_stereo_equalizer;

static void px_equalizer_mono_add_band(px_mono_equalizer* equalizer, float frequency, float quality, float gain, BiquadFilterType type)
{
    px_mono_biquad newFilter;
    px_biquad_
}
static void px_stereo_equalizer_add_band(px_stereo_equalizer* stereoEqualizer);
