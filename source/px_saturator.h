#include "px_globals.h"
#include "px_memory.h"


#ifndef PX_SATURATOR_H
#define PX_SATURATOR_H

typedef enum
{
    ARCTANGENT,
    TANGENT
} SATURATION_CURVE;

typedef struct
{
    float drive;
    SATURATION_CURVE curve;
} px_saturator;


// ----------------------------------------------------------------------------------------------------


static void px_saturator_initialize(px_saturator* saturator, SATURATION_CURVE curve);
static px_saturator* px_saturator_create(SATURATION_CURVE curve);
static void px_saturator_destroy(px_saturator* saturator);

static void px_saturator_set_drive(px_saturator* saturator, float drive);
static void px_saturator_set_curve(px_saturator* saturator, SATURATION_CURVE curve);
static void px_saturator_mono_process(px_saturator* saturator, float* input);
static void px_saturator_stereo_process(px_saturator* saturator, float* input_left, float* input_right);

static inline float px_saturate_arctangent(float input, float drive);
static inline float px_saturate_tangent(float input, float drive);


// ----------------------------------------------------------------------------------------------------


static void px_saturator_initialize(px_saturator* saturator, SATURATION_CURVE curve)
{
    assert(saturator);
    saturator->drive = 0.f;
    saturator->curve = curve;
}

static px_saturator* px_saturator_create(SATURATION_CURVE curve)
{
    px_saturator* saturator = (px_saturator*)px_malloc(sizeof(px_saturator));
    if (saturator)
    {
        px_saturator_initialize(saturator, curve);
        return saturator;
    }
    else return NULL;
}

static void px_saturator_destroy(px_saturator* saturator)
{
    if (saturator)
	px_free(saturator);
}

static void px_saturator_set_drive(px_saturator* saturator, float drive)
{
    assert(saturator);
    saturator->drive = drive;
}

static void px_saturator_set_curve(px_saturator* saturator, SATURATION_CURVE curve)
{
    assert(saturator);
    saturator->curve = curve;
}

static void px_saturator_mono_process(px_saturator* saturator, float* input)
{
    px_assert(saturator, input);
    switch (saturator->curve)
    {	
	case ARCTANGENT:
		*input = px_saturate_arctangent(*input, saturator->drive);
		break;

	case TANGENT:
		*input = px_saturate_tangent(*input, saturator->drive);	
    		break;
    }
}

static void px_saturator_stereo_process(px_saturator* saturator, float* input_left, float* input_right)
{
    px_assert(saturator, input_left, input_right);
    switch (saturator->curve)
    {	
	case ARCTANGENT:
		*input_left = px_saturate_arctangent(*input_left, saturator->drive);
		*input_right = px_saturate_arctangent(*input_right, saturator->drive);
		break;

	case TANGENT:
		*input_left = px_saturate_tangent(*input_left, saturator->drive);
    		*input_right = px_saturate_tangent(*input_right, saturator->drive);
		break;
    }
}

static inline float px_saturate_arctangent(float input, float drive)
{
    return atan(input * dB2lin(drive));
}

static inline float px_saturate_tangent(float input, float drive)
{
    return tanh(input * dB2lin(drive));
}

#endif
