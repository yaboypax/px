#include "px_globals.h"

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

