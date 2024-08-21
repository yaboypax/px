#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "px_biquad.h"
#include "px_memory.h"


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

    px_mono_biquad sidechain_filter;
} px_mono_compressor;

typedef struct 
{
    px_mono_compressor left;
    px_mono_compressor right;
   
    px_compressor_parameters parameters; 
    px_stereo_biquad sidechain_filter;
} px_stereo_compressor;

// API functions
// ----------------------------------------------------------------------------------------------------------------------
// mono
static px_mono_compressor* px_compressor_mono_create(float in_sample_rate);
static void px_compressor_mono_destroy(px_mono_compressor* compressor);

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

static px_stereo_compressor* px_compressor_stereo_create(float in_sample_rate);
static void px_compressor_stereo_destroy(px_stereo_compressor* stereo_compressor);

static void px_compressor_stereo_process(px_stereo_compressor* stereo_compressor, float* input_left, float* input_right);
static void px_compressor_stereo_initialize(px_stereo_compressor* stereo_compressor, float in_sample_rate);

static void px_compressor_stereo_set_parameters(px_stereo_compressor* stereo_compressor, px_compressor_parameters in_parameters);
static void px_compressor_stereo_set_threshold(px_stereo_compressor* stereo_compressor, float in_threshold);
static void px_compressor_stereo_set_ratio(px_stereo_compressor* stereo_compressor, float in_ratio);
static void px_compressor_stereo_set_knee(px_stereo_compressor* stereo_compressor, float in_knee_width);
static void px_compressor_stereo_set_attack(px_stereo_compressor* stereo_compressor, float in_attack);
static void px_compressor_stereo_set_release(px_stereo_compressor* stereo_compressor, float in_release);
static void px_compressor_stereo_set_makeup_gain(px_stereo_compressor* stereo_compressor, float in_gain);

static void px_compressor_stereo_set_sidechain_frequency(px_stereo_compressor* compressor, float in_frequency);
static void px_compressor_stereo_set_sidechain_quality(px_stereo_compressor* compressor, float in_quality);
static void px_compressor_stereo_set_sidechain_gain(px_stereo_compressor* compressor, float in_gain);
static void px_compressor_stereo_set_sidechain_type(px_stereo_compressor* compressor, BIQUAD_FILTER_TYPE in_type);

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
    if (in_sample_rate < 40000.f) //test for validity probably a better way
	return NULL;

    px_mono_compressor* compressor = (px_mono_compressor*)px_malloc(sizeof(px_mono_compressor));
    if (compressor)
	px_compressor_mono_initialize(compressor, in_sample_rate);
	return compressor;

}


static px_stereo_compressor* px_compressor_stereo_create(float in_sample_rate)
{
    if (in_sample_rate < 40000.f) //test for validity probably a better way
	return NULL;

    px_stereo_compressor* stereo_compressor = (px_stereo_compressor*)px_malloc(sizeof(px_stereo_compressor));
    if (stereo_compressor)
	px_compressor_stereo_initialize(stereo_compressor, in_sample_rate);
	return stereo_compressor;

}

static void px_compressor_mono_destroy(px_mono_compressor* compressor)
{
    if (compressor)
	px_free(compressor);
}

static void px_compressor_stereo_destroy(px_stereo_compressor* stereo_compressor)
{
    if (stereo_compressor)
	px_free(stereo_compressor);
}

static void px_compressor_mono_process(px_mono_compressor* compressor, float* input)
{
    // check the caclulate_envelope function
    assert(compressor);
    //sidechain eq
    float sidechain = *input;
    px_biquad_mono_process(&compressor->sidechain_filter, &sidechain);
    *input = px_compressor_compress(compressor, *input, sidechain);
}

static void px_compressor_stereo_process(px_stereo_compressor* stereo_compressor, float* input_left, float* input_right)
{
    assert(stereo_compressor);

    //sidechain eq
    float sidechain_left = *input_left;
    float sidechain_right = *input_right;

    px_biquad_stereo_process(&stereo_compressor->sidechain_filter, &sidechain_left, &sidechain_right);

    float input_absolute_left = fabsf(sidechain_left);
    float input_absolute_right = fabsf(sidechain_right); /* put here: rms smoothing */

   //mono sum
   float input_link = fabsf(fmaxf(input_absolute_left, input_absolute_right));

  *input_left = px_compressor_compress(&stereo_compressor->left, *input_left, input_link);
  *input_right = px_compressor_compress(&stereo_compressor->right, *input_right, input_link);

}



static void px_compressor_mono_initialize(px_mono_compressor* compressor, float in_sample_rate)
{
    assert(compressor);

    px_mono_biquad filter;
    px_biquad_mono_initialize(&filter, in_sample_rate, BIQUAD_HIGHPASS);
    px_biquad_mono_set_frequency(&filter, 0.f);

    px_compressor_parameters new_parameters = INITIALIZED_PARAMETERS;
    compressor->parameters = new_parameters;

    compressor->attack.sample_rate = in_sample_rate;
    compressor->release.sample_rate = in_sample_rate;

    compressor->attack.time_constant = 10.f;
    compressor->release.time_constant = 100.f;

    px_envelope_detector_calculate_coefficient(&compressor->attack);
    px_envelope_detector_calculate_coefficient(&compressor->release);
}

static void px_compressor_stereo_initialize(px_stereo_compressor* stereo_compressor, float in_sample_rate)
{
    assert(stereo_compressor);
    
    px_stereo_biquad filter;
    px_biquad_stereo_initialize(&filter, in_sample_rate, BIQUAD_HIGHPASS);
    px_biquad_stereo_set_frequency(&filter, 0.f);
    stereo_compressor->sidechain_filter = filter;

    px_compressor_parameters new_parameters = INITIALIZED_PARAMETERS; 
    stereo_compressor->parameters = new_parameters;


    px_compressor_mono_initialize(&stereo_compressor->left, in_sample_rate);
    px_compressor_mono_initialize(&stereo_compressor->right, in_sample_rate);

}

static void px_compressor_mono_set_parameters(px_mono_compressor* compressor, px_compressor_parameters in_parameters)
{
    assert(compressor);
    compressor->parameters = in_parameters;
}

static void px_compressor_stereo_set_parameters(px_stereo_compressor* stereo_compressor, px_compressor_parameters in_parameters)
{
    assert(stereo_compressor);
    stereo_compressor->parameters = in_parameters;
    px_compressor_mono_set_parameters(&stereo_compressor->left, in_parameters);
    px_compressor_mono_set_parameters(&stereo_compressor->right, in_parameters);
}


static void px_compressor_mono_set_threshold(px_mono_compressor* compressor, float in_threshold)
{
    assert(compressor);
    compressor->parameters.threshold = in_threshold;
}

static void px_compressor_stereo_set_threshold(px_stereo_compressor* stereo_compressor, float in_threshold)
{
    assert(stereo_compressor);
    stereo_compressor->parameters.threshold = in_threshold;
    px_compressor_mono_set_threshold(&stereo_compressor->left, in_threshold);
    px_compressor_mono_set_threshold(&stereo_compressor->right, in_threshold);
}

static void px_compressor_mono_set_ratio(px_mono_compressor* compressor, float in_ratio)
{
    assert(compressor);
    compressor->parameters.ratio = in_ratio;
}

static void px_compressor_stereo_set_ratio(px_stereo_compressor* stereo_compressor, float in_ratio)
{
    assert(stereo_compressor);
    stereo_compressor->parameters.ratio = in_ratio;
    px_compressor_mono_set_ratio(&stereo_compressor->left, in_ratio);
    px_compressor_mono_set_ratio(&stereo_compressor->right, in_ratio);
}

static void px_compressor_mono_set_knee(px_mono_compressor* compressor, float in_knee_width)
{
    assert(compressor);
    compressor->parameters.knee_width = in_knee_width;
}

static void px_compressor_stereo_set_knee(px_stereo_compressor* stereo_compressor, float in_knee_width)
{
    assert(stereo_compressor);
    stereo_compressor->parameters.knee_width = in_knee_width;
    px_compressor_mono_set_knee(&stereo_compressor->left, in_knee_width);
    px_compressor_mono_set_knee(&stereo_compressor->right, in_knee_width);
}

static void px_compressor_mono_set_attack(px_mono_compressor* compressor, float in_attack)
{
    assert(compressor);
    compressor->parameters.attack = in_attack;
    compressor->attack.time_constant = in_attack;
    px_envelope_detector_calculate_coefficient(&compressor->attack);
}

static void px_compressor_stereo_set_attack(px_stereo_compressor* stereo_compressor, float in_attack)
{
    assert(stereo_compressor);
    stereo_compressor->parameters.attack = in_attack;
    px_compressor_mono_set_attack(&stereo_compressor->left, in_attack);
    px_compressor_mono_set_attack(&stereo_compressor->right, in_attack);
}


static void px_compressor_mono_set_release(px_mono_compressor* compressor, float in_release)
{
    assert(compressor);
    compressor->parameters.release = in_release;
    compressor->release.time_constant = in_release;
    px_envelope_detector_calculate_coefficient(&compressor->release);
}

static void px_compressor_stereo_set_release(px_stereo_compressor* stereo_compressor, float in_release)
{
    assert(stereo_compressor);
    stereo_compressor->parameters.release = in_release;
    px_compressor_mono_set_release(&stereo_compressor->left, in_release);
    px_compressor_mono_set_release(&stereo_compressor->right, in_release);
}

static void px_compressor_mono_set_makeup_gain(px_mono_compressor* compressor, float in_gain)
{
    assert(compressor);
    compressor->parameters.makeup_gain = in_gain;
}

static void px_compressor_stereo_set_makeup_gain(px_stereo_compressor* stereo_compressor, float in_gain)
{
    assert(stereo_compressor);
    stereo_compressor->parameters.makeup_gain = in_gain;
    px_compressor_mono_set_makeup_gain(&stereo_compressor->left, in_gain);
    px_compressor_mono_set_makeup_gain(&stereo_compressor->right, in_gain);
}
 
static void px_compressor_mono_set_sidechain_frequency(px_mono_compressor* compressor, float in_frequency)
{
    assert(compressor);
    px_biquad_mono_set_frequency(&compressor->sidechain_filter, in_frequency);
}

static void px_compressor_stereo_set_sidechain_frequency(px_stereo_compressor* stereo_compressor, float in_frequency)
{
    assert(stereo_compressor);
    px_biquad_stereo_set_frequency(&stereo_compressor->sidechain_filter, in_frequency);
}

 
static void px_compressor_mono_set_sidechain_quality(px_mono_compressor* compressor, float in_quality)
{
    assert(compressor);
    px_biquad_mono_set_quality(&compressor->sidechain_filter, in_quality);
}

static void px_compressor_stereo_set_sidechain_quality(px_stereo_compressor* stereo_compressor, float in_quality)
{
    assert(stereo_compressor);
    px_biquad_stereo_set_quality(&stereo_compressor->sidechain_filter, in_quality);
}

static void px_compressor_mono_set_sidechain_gain(px_mono_compressor* compressor, float in_gain)
{
    assert(compressor);
    px_biquad_mono_set_gain(&compressor->sidechain_filter, in_gain);
}

static void px_compressor_stereo_set_sidechain_gain(px_stereo_compressor* stereo_compressor, float in_gain)
{
    assert(stereo_compressor);
    px_biquad_stereo_set_gain(&stereo_compressor->sidechain_filter, in_gain);
}

static void px_compressor_mono_set_sidechain_type(px_mono_compressor* compressor, BIQUAD_FILTER_TYPE in_type)
{
    assert(compressor);
    px_biquad_mono_set_type(&compressor->sidechain_filter, in_type);
}

static void px_compressor_stereo_set_sidechain_type(px_stereo_compressor* stereo_compressor, BIQUAD_FILTER_TYPE in_type)
{
    assert(stereo_compressor);
    px_biquad_stereo_set_type(&stereo_compressor->sidechain_filter, in_type);
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





// px_compressor* px_create_compressor(float sample_rate)
// {
//     px_compressor
// }
