#include "stdio.h"
#include <math.h>
#include <stdbool.h>
#include <assert.h>

#include "px_globals.h"

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
} BiquadFilterType;

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
   BiquadFilterType type;
} px_biquad_parameters;

typedef struct 
{
   px_biquad_coefficients coefficients;
   px_biquad_parameters parameters;
} px_mono_biquad;

typedef struct
{
   px_mono_biquad left;
   px_mono_biquad right;
} px_stereo_biquad;


// api functions
// -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// mono
static void px_biquad_mono_process_sample(px_mono_biquad* biquad, float* input);
static void px_biquad_mono_initialize(px_mono_biquad* biquad, float sampleRate, BiquadFilterType type);

static void px_biquad_mono_set_frequency(px_mono_biquad* biquad, float inFrequency);
static void px_biquad_mono_set_quality(px_mono_biquad* biquad, float inQuality);
static void px_biquad_mono_set_gain(px_mono_biquad* biquad, float inGain);
static void px_biquad_mono_set_type(px_mono_biquad* biquad, BiquadFilterType inType);

// stereo
static void px_biquad_stereo_process_sample(px_stereo_biquad* stereoBiquad, float* in_left, float* in_right);
static void px_biquad_stereo_initialize(px_stereo_biquad* stereoBiquad, float sampleRate, BiquadFilterType type);

static void px_biquad_stereo_set_frequency(px_stereo_biquad* biquad, float inFrequency);
static void px_biquad_stereo_set_quality(px_stereo_biquad* biquad, float inQuality);
static void px_biquad_stereo_set_gain(px_stereo_biquad* biquad, float inGain);
static void px_biquad_stereo_set_type(px_stereo_biquad* biquad, BiquadFilterType inType);
/
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// inline functions

static inline float px_biquad_filter(px_mono_biquad* biquad, float input);
static inline void px_biquad_update_coefficients(const px_biquad_parameters parameters, px_biquad_coefficients* coefficients);

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void px_biquad_mono_process_sample(px_mono_biquad* biquad, float* input)
{
    assert(biquad);
    float mono = *input;
    *input = px_biquad_filter(biquad, mono);
}

static void px_biquad_stereo_process_sample(px_stereo_biquad* stereoBiquad, float* in_left, float* in_right)
{
    assert(stereoBiquad);
    float left = *in_left;
    float right = *in_right;

    *in_left = px_biquad_filter(&stereoBiquad->left, left);
    *in_right = px_biquad_filter(&stereoBiquad->right, right);
}

static void px_biquad_mono_initialize(px_mono_biquad* biquad, float sampleRate, BiquadFilterType type)
{
    assert(biquad);
    px_biquad_parameters parameters = { sampleRate, 100.f, 0.5f, type };
    px_biquad_coefficients coefficients = { 1.f, 0.f, 0.0f, 0.0f, 0.0f };
    px_biquad_update_coefficients(parameters, &coefficients);

    biquad->parameters = parameters;
    biquad->coefficients = coefficients;
}

static void px_biquad_stereo_initialize(px_stereo_biquad* stereoBiquad, float sampleRate, BiquadFilterType type)
{
    assert(stereoBiquad);
    px_biquad_mono_initialize(&stereoBiquad->left, sampleRate, type);
    px_biquad_mono_initialize(&stereoBiquad->right, sampleRate, type);
}

static void px_biquad_mono_set_frequency(px_mono_biquad* biquad, float inFrequency)
{
    assert(biquad);
    biquad->parameters.frequency = inFrequency;
    px_biquad_update_coefficients(biquad->parameters, &biquad->coefficients);
}

static void px_biquad_stereo_set_frequency(px_stereo_biquad* stereoBiquad, float inFrequency)
{
    assert(stereoBiquad);
    px_biquad_mono_set_frequency(&stereoBiquad->left, inFrequency);
    px_biquad_mono_set_frequency(&stereoBiquad->right, inFrequency);
}

static void px_biquad_mono_set_quality(px_mono_biquad* biquad, float inQuality)
{
    assert(biquad);
    biquad->parameters.quality = inQuality;
    px_biquad_update_coefficients(biquad->parameters, &biquad->coefficients);
}

static void px_biquad_stereo_set_quality(px_stereo_biquad* stereoBiquad, float inQuality)
{
    asssert(stereoBiquad);
    px_biquad_mono_set_quality(&stereoBiquad->left, inQuality);
    px_biquad_mono_set_quality(&stereoBiquad->right, inQuality);
}

static void px_biquad_mono_set_gain(px_mono_biquad* biquad, float inGain)
{
    assert(biquad);
    biquad->parameters.gain = inGain;
    px_biquad_update_coefficients(biquad->parameters, &biquad->coefficients);
}

static void px_biquad_stereo_set_gain(px_stereo_biquad* stereoBiquad, float inGain)
{
    asssert(stereoBiquad);
    px_biquad_mono_set_gain(&stereoBiquad->left, inGain);
    px_biquad_mono_set_gain(&stereoBiquad->right, inGain);
}

static void px_biquad_mono_set_type(px_mono_biquad* biquad, BiquadFilterType inType)
{
    assert(biquad);
    biquad->parameters.type = inType;
    px_biquad_update_coefficients(biquad->parameters, &biquad->coefficients);
}

static void px_biquad_stereo_set_type(px_stereo_biquad* stereoBiquad, BiquadFilterType inType)
{
    asssert(stereoBiquad);
    px_biquad_mono_set_type(&stereoBiquad->left, inType);
    px_biquad_mono_set_type(&stereoBiquad->right, inType);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static inline float px_biquad_filter(px_mono_biquad* biquad, float input)
{
    float out = input * biquad->coefficients.a0 + biquad->coefficients.z1;
    biquad->coefficients.z1 = input * biquad->coefficients.a1 + biquad->coefficients.z2 - biquad->coefficients.b1 * out;
    biquad->coefficients.z2 = input * biquad->coefficients.a2 - biquad->coefficients.b2 * out;
    return out;
}

static inline void px_biquad_update_coefficients(const px_biquad_parameters parameters, px_biquad_coefficients* coefficients)
{
    float a0 = coefficients->a0;
    float a1 = coefficients->a1;
    float a2 = coefficients->a2;
    float b1 = coefficients->b1;
    float b2 = coefficients->b2;

    if (parameters.frequency <= 0 || parameters.frequency != parameters.frequency)
    {
        a0 = 1;
        a1 = 0;
        a2 = 0;
        b1 = 0;
        b2 = 0;
        return;
    }

    float norm;
    float V = pow(10, fabs(parameters.gain) / 20.0);
    float K = tan(PI * (parameters.frequency / parameters.sample_rate));

    //  printf("%d", V);
    //  printf("%d", K);

    switch (parameters.type)
    {
    case BIQUAD_LOWPASS:
        norm = 1 / (1 + K / parameters.quality + K * K);
        a0 = K * K * norm;
        a1 = 2 * a0;
        a2 = a0;
        b1 = 2 * (K * K - 1) * norm;
        b2 = (1 - K / parameters.quality + K * K) * norm;
        break;

    case BIQUAD_HIGHPASS:
        norm = 1 / (1 + K / parameters.quality + K * K);
        a0 = 1 * norm;
        a1 = -2 * a0;
        a2 = a0;
        b1 = 2 * (K * K - 1) * norm;
        b2 = (1 - K / parameters.quality + K * K) * norm;
        break;

    case BIQUAD_BANDPASS:
        norm = 1 / (1 + K / parameters.quality + K * K);
        a0 = K / parameters.quality * norm;
        a1 = 0;
        a2 = -a0;
        b1 = 2 * (K * K - 1) * norm;
        b2 = (1 - K / parameters.quality + K * K) * norm;
        break;

    case BIQUAD_NOTCH:
        norm = 1 / (1 + K / parameters.quality + K * K);
        a0 = (1 + K * K) * norm;
        a1 = 2 * (K * K - 1) * norm;
        a2 = a0;
        b1 = a1;
        b2 = (1 - K / parameters.quality + K * K) * norm;
        break;

    case BIQUAD_PEAK:
        if (parameters.gain >= 0)
        { // boost
            norm = 1 / (1 + K / parameters.quality + K * K);
            a0 = (1 + K / parameters.quality * V + K * K) * norm;
            a1 = 2 * (K * K - 1) * norm;
            a2 = (1 - K / parameters.quality * V + K * K) * norm;
            b1 = a1;
            b2 = (1 - K / parameters.quality + K * K) * norm;
        }
        else
        { // cut
            norm = 1 / (1 + K / parameters.quality * V + K * K);
            a0 = (1 + K / parameters.quality + K * K) * norm;
            a1 = 2 * (K * K - 1) * norm;
            a2 = (1 - K / parameters.quality + K * K) * norm;
            b1 = a1;
            b2 = (1 - K / parameters.quality * V + K * K) * norm;
        }
        break;

    case BIQUAD_LOWSHELF:
        if (parameters.gain >= 0)
        { // boost
            norm = 1 / (1 + K / parameters.quality + K * K);
            a0 = (1 + sqrt(V) * K / parameters.quality + V * K * K) * norm;
            a1 = 2 * (V * K * K - 1) * norm;
            a2 = (1 - sqrt(V) * K / parameters.quality + V * K * K) * norm;
            b1 = 2 * (K * K - 1) * norm;
            b2 = (1 - K / parameters.quality + K * K) * norm;
        }
        else
        { // cut
            norm = 1 / (1 + sqrt(V) * K / parameters.quality + V * K * K);
            a0 = (1 + K / parameters.quality + K * K) * norm;
            a1 = 2 * (K * K - 1) * norm;
            a2 = (1 - K / parameters.quality + K * K) * norm;
            b1 = 2 * (V * K * K - 1) * norm;
            b2 = (1 - sqrt(V) * K / parameters.quality + V * K * K) * norm;
        }
        break;

    case BIQUAD_HIGHSHELF:
        if (parameters.gain >= 0)
        { // boost
            norm = 1 / (1 + K / parameters.quality + K * K);
            a0 = (V + sqrt(V) * K / parameters.quality + K * K) * norm;
            a1 = 2 * (K * K - V) * norm;
            a2 = (V - sqrt(V) * K / parameters.quality + K * K) * norm;
            b1 = 2 * (K * K - 1) * norm;
            b2 = (1 - K / parameters.quality + K * K) * norm;
        }
        else
        { // cut
            norm = 1 / (V + sqrt(V) * K / parameters.quality + K * K);
            a0 = (1 + K / parameters.quality + K * K) * norm;
            a1 = 2 * (K * K - 1) * norm;
            a2 = (1 - K / parameters.quality + K * K) * norm;
            b1 = 2 * (K * K - V) * norm;
            b2 = (V - sqrt(V) * K / parameters.quality + K * K) * norm;
        }
        break;

    case BIQUAD_LOWSHELF_NOQ:
        if (parameters.gain >= 0)
        { // boost
            norm = 1 / (1 + sqrt(2) * K + K * K);
            a0 = (1 + sqrt(2 * V) * K + V * K * K) * norm;
            a1 = 2 * (V * K * K - 1) * norm;
            a2 = (1 - sqrt(2 * V) * K + V * K * K) * norm;
            b1 = 2 * (K * K - 1) * norm;
            b2 = (1 - sqrt(2) * K + K * K) * norm;
        }
        else
        { // cut
            norm = 1 / (1 + sqrt(2 * V) * K + V * K * K);
            a0 = (1 + sqrt(2) * K + K * K) * norm;
            a1 = 2 * (K * K - 1) * norm;
            a2 = (1 - sqrt(2) * K + K * K) * norm;
            b1 = 2 * (V * K * K - 1) * norm;
            b2 = (1 - sqrt(2 * V) * K + V * K * K) * norm;
        }
        break;

    case BIQUAD_HIGHSHELF_NOQ:
        if (parameters.gain >= 0)
        { // boost
            norm = 1 / (1 + sqrt(2) * K + K * K);
            a0 = (V + sqrt(2 * V) * K + K * K) * norm;
            a1 = 2 * (K * K - V) * norm;
            a2 = (V - sqrt(2 * V) * K + K * K) * norm;
            b1 = 2 * (K * K - 1) * norm;
            b2 = (1 - sqrt(2) * K + K * K) * norm;
        }
        else
        { // cut
            norm = 1 / (V + sqrt(2 * V) * K + K * K);
            a0 = (1 + sqrt(2) * K + K * K) * norm;
            a1 = 2 * (K * K - 1) * norm;
            a2 = (1 - sqrt(2) * K + K * K) * norm;
            b1 = 2 * (K * K - V) * norm;
            b2 = (V - sqrt(2 * V) * K + K * K) * norm;
        }
        break;

    case BIQUAD_ALLPASS:
        norm = 1 / (1 + K / parameters.quality + K * K);
        a0 = (1 - K / parameters.quality + K * K) * norm;
        a1 = 2 * (K * K - 1) * norm;
        a2 = 1;
        b1 = a1;
        b2 = a0;
        break;

    case BIQUAD_NONE:
        a0 = 1;
        a1 = 0;
        a2 = 0;
        b1 = 0;
        b2 = 0;
        break;
    }

    coefficients->a0 = a0;
    coefficients->a1 = a1;
    coefficients->a2 = a2;
    coefficients->b1 = b1;
    coefficients->b2 = b2;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------



