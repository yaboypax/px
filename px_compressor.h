#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "px_globals.h"

typedef struct
{
    float sampleRate;
    float timeConstant;
    float coefficient;

} px_envelope_detector;

typedef struct
{
    float threshold;
    float ratio;
    float env;              // over-threshold envelope
    float kneeWidth;
    float makeupGain;
} px_compressor_parameters;

typedef struct
{
    px_compressor_parameters parameters;
    px_envelope_detector attack;
    px_envelope_detector release;
} px_mono_compressor;

typedef struct 
{
    px_mono_compressor left;
    px_mono_compressor right;
} px_stereo_compressor;

// API functions
// ----------------------------------------------------------------------------------------------------------------------
// mono
static void px_compressor_mono_process(px_mono_compressor* compressor, float* input, bool isStereo, float linkedSignal);
static void px_compressor_mono_initialize(px_mono_compressor* compressor, float inSampleRate);

static void px_compressor_mono_set_parameters(px_mono_compressor* compressor, px_compressor_parameters inParameters);
static void px_compressor_mono_set_threshold(px_mono_compressor* compressor, float inThreshold);
static void px_compressor_mono_set_ratio(px_mono_compressor* compressor, float inRatio);
static void px_compressor_mono_set_knee(px_mono_compressor* compressor, float inKneeWidth);
static void px_compressor_mono_set_attack(px_mono_compressor* compressor, float inAttack);
static void px_compressor_mono_set_release(px_mono_compressor* compressor, float inRelease);
static void px_compressor_mono_set_makeup_gain(px_mono_compressor* compressor, float inGain);

// stereo
static void px_compressor_stereo_process(px_stereo_compressor* stereoCompressor, float* inputLeft, float* inputRight);
static void px_compressor_stereo_initialize(px_stereo_compressor* stereoCompressor, float inSampleRate);

static void px_compressor_stereo_set_parameters(px_stereo_compressor* stereoCompressor, px_compressor_parameters inParameters);
static void px_compressor_stereo_set_threshold(px_stereo_compressor* stereoCompressor, float inThreshold);
static void px_compressor_stereo_set_ratio(px_stereo_compressor* stereoCompressor, float inRatio);
static void px_compressor_stereo_set_knee(px_stereo_compressor* stereoCompressor, float inKneeWidth);
static void px_compressor_stereo_set_attack(px_stereo_compressor* stereoCompressor, float inAttack);
static void px_compressor_stereo_set_release(px_stereo_compressor* stereoCompressor, float inRelease);
static void px_compressor_stereo_set_makeup_gain(px_stereo_compressor* stereoCompressor, float inGain);
// ----------------------------------------------------------------------------------------------------------------------

// inline functions 
// ----------------------------------------------------------------------------------------------------------------------

static inline void px_envelope_detector_calculate_coefficient(px_envelope_detector* envelope)
{
    envelope->coefficient = exp(-1000.f / (envelope->timeConstant * envelope->sampleRate));
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
    float kneeStart = compressor->parameters.threshold - compressor->parameters.kneeWidth / 2.0;
    float kneeEnd = compressor->parameters.threshold + compressor->parameters.kneeWidth / 2.0;

    
    float gain = 1.0;
        // no compression
    if (overdB <= kneeStart) {
        gain = 1.0;
    }
    else if (overdB >= kneeEnd) {
        // Full compression above the knee
        float reducedLevel = overdB / compressor->parameters.ratio;
        gain = dB2lin(-(overdB - reducedLevel));
    }
    else {
        // soft knee
        // Within the knee, interpolate the gain reduction
        float blend = (overdB - kneeStart) / compressor->parameters.kneeWidth; 
        float uncompressedGain = 1.0;
        float reducedLevel = overdB / compressor->parameters.ratio;
        float compressedGain = dB2lin(-(overdB - reducedLevel));
        gain = uncompressedGain + blend * (compressedGain - uncompressedGain);
    }

    return gain;
}

// --------------------------------------------------------------------------------------------------------

static void px_compressor_mono_process(px_mono_compressor* compressor, float* input, bool isStereo, float linkedSignal)
{
    // check the caclulate_envelope function
    assert(!isnan(compressor->parameters.env));

    if (!isStereo)
    {
        linkedSignal = *input;
    }

    linkedSignal += DC_OFFSET;   // avoid log( 0 )
    float keydB = lin2dB(linkedSignal); 

    //threshold
    float overdB = keydB - compressor->parameters.threshold;
    if (overdB < 0.f)
        overdB = 0.f;
    
    //attack/release
    overdB += DC_OFFSET;  // avoid denormal
    px_compressor_calculate_envelope(compressor, overdB, &compressor->parameters.env);
    overdB = compressor->parameters.env - DC_OFFSET;

    //transfer function
    float gainReduction;
    if (compressor->parameters.kneeWidth > 0.f)
    {
        gainReduction = px_compressor_calculate_knee(compressor, overdB); //return linear
    }
    else
    {
        gainReduction = dB2lin(-overdB);
    }

    *input *= gainReduction;

    //makeup gain
    float makeup = dB2lin(compressor->parameters.makeupGain);
    *input *= makeup;
}

static void px_compressor_stereo_process(px_stereo_compressor* stereoCompressor, float* inputLeft, float* inputRight)
{
    assert(stereoCompressor);

    /*
        put here:

        sidechain eq process
    
    */

    float inputAbsoluteLeft = fabsf(*inputLeft);
    float inputAbsoluteRight = fabsf(*inputRight);

    /*
        put here:

        rms smooth envelope
    
    */

   //mono sum
   float inputLink = fabsf(fmaxf(inputAbsoluteLeft, inputAbsoluteRight));

   px_compressor_mono_process(&stereoCompressor->left, inputLeft, true, inputLink);
   px_compressor_mono_process(&stereoCompressor->right, inputRight, true, inputLink);

}



static void px_compressor_mono_initialize(px_mono_compressor* compressor, float inSampleRate)
{
    assert(compressor);

    px_compressor_parameters newParameters = { 0.f, 1.f, 0.f, 0.f};
    compressor->parameters = newParameters;

    compressor->attack.sampleRate = inSampleRate;
    compressor->release.sampleRate = inSampleRate;

    compressor->attack.timeConstant = 10.f;
    compressor->release.timeConstant = 100.f;

    px_envelope_detector_calculate_coefficient(&compressor->attack);
    px_envelope_detector_calculate_coefficient(&compressor->release);
}

static void px_compressor_stereo_initialize(px_stereo_compressor* stereoCompressor, float inSampleRate)
{
    assert(stereoCompressor);
    px_compressor_mono_initialize(&stereoCompressor->left, inSampleRate);
    px_compressor_mono_initialize(&stereoCompressor->right, inSampleRate);

}

static void px_compressor_mono_set_parameters(px_mono_compressor* compressor, px_compressor_parameters inParameters)
{
    assert(compressor);
    compressor->parameters = inParameters;
}

static void px_compressor_stereo_set_parameters(px_stereo_compressor* stereoCompressor, px_compressor_parameters inParameters)
{
    assert(stereoCompressor);
    px_compressor_mono_set_parameters(&stereoCompressor->left, inParameters);
    px_compressor_mono_set_parameters(&stereoCompressor->right, inParameters);
}


static void px_compressor_mono_set_threshold(px_mono_compressor* compressor, float inThreshold)
{
    assert(compressor);
    compressor->parameters.threshold = inThreshold;
}

static void px_compressor_stereo_set_threshold(px_stereo_compressor* stereoCompressor, float inThreshold)
{
    assert(stereoCompressor);
    px_compressor_mono_set_threshold(&stereoCompressor->left, inThreshold);
    px_compressor_mono_set_threshold(&stereoCompressor->right, inThreshold);
}

static void px_compressor_mono_set_ratio(px_mono_compressor* compressor, float inRatio)
{
    assert(compressor);
    compressor->parameters.ratio = inRatio;
}

static void px_compressor_stereo_set_ratio(px_stereo_compressor* stereoCompressor, float inRatio)
{
    assert(stereoCompressor);
    px_compressor_mono_set_ratio(&stereoCompressor->left, inRatio);
    px_compressor_mono_set_ratio(&stereoCompressor->right, inRatio);
}

static void px_compressor_mono_set_knee(px_mono_compressor* compressor, float inKneeWidth)
{
    assert(compressor);
    compressor->parameters.kneeWidth = inKneeWidth;
}

static void px_compressor_stereo_set_knee(px_stereo_compressor* stereoCompressor, float inKneeWidth)
{
    assert(stereoCompressor);
    px_compressor_mono_set_knee(&stereoCompressor->left, inKneeWidth);
    px_compressor_mono_set_knee(&stereoCompressor->right, inKneeWidth);
}

static void px_compressor_mono_set_attack(px_mono_compressor* compressor, float inAttack)
{
    assert(compressor);
    compressor->attack.timeConstant = inAttack;
    px_envelope_detector_calculate_coefficient(&compressor->attack);
}

static void px_compressor_stereo_set_attack(px_stereo_compressor* stereoCompressor, float inAttack)
{
    assert(stereoCompressor);
    px_compressor_mono_set_attack(&stereoCompressor->left, inAttack);
    px_compressor_mono_set_attack(&stereoCompressor->right, inAttack);
}


static void px_compressor_mono_set_release(px_mono_compressor* compressor, float inRelease)
{
    assert(compressor);
    compressor->release.timeConstant = inRelease;
    px_envelope_detector_calculate_coefficient(&compressor->release);
}

static void px_compressor_stereo_set_release(px_stereo_compressor* stereoCompressor, float inRelease)
{
    assert(stereoCompressor);
    px_compressor_mono_set_release(&stereoCompressor->left, inRelease);
    px_compressor_mono_set_release(&stereoCompressor->right, inRelease);
}

static void px_compressor_mono_set_makeup_gain(px_mono_compressor* compressor, float inGain)
{
    assert(compressor);
    compressor->parameters.makeupGain = inGain;
}

static void px_compressor_stereo_set_makeup_gain(px_stereo_compressor* stereoCompressor, float inGain)
{
    assert(stereoCompressor);
    px_compressor_mono_set_makeup_gain(&stereoCompressor->left, inGain);
    px_compressor_mono_set_makeup_gain(&stereoCompressor->right, inGain);
}






// px_compressor* px_create_compressor(float sampleRate)
// {
//     px_compressor
// }
