#include "px_biquad.h"

#define MAX_BANDS 8


typedef struct {

    px_mono_biquad filterBank[MAX_BANDS];
} px_mono_equalizer;

typedef struct {
    px_stereo_biquad stereoFilterBank[MAX_BANDS];
} px_stereo_equalizer;


