#include "px_globals.h"
#include "px_equalizer.h"
#include "px_memory.h"

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

#define PX_DUAL_MONO true
#define PX_STEREO    false
#define PX_MID_SIDE  false

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
