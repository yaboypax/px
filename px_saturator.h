#include <math.h>

typedef enum
{
    SINUSOIDAL,
    TANGENT
} SATURATION_CURVE;

typedef struct
{
    float drive;
    SATURATION_CURVE curve;
} px_saturator;

static void px_saturator_initialize(px_saturator* saturator, SATURATION_CURVE curve)
{
    assert(saturator)
    saturator->curve = curve;
}

static px_saturator* px_saturator_create(SATURATION_CURVE curve)
{
    px_saturator* saturator = (px_saturator*)malloc(sizeof* px_saturator);
    if (saturator)
	px_saturatior_initialize(saturator, curve);
	return saturator;
}

static void px_saturator_destroy(px_saturator* saturator)
{
    if (saturator)
	free(saturator);
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

static void px_mono_saturator_process(px_saturator* saturator, float* input)
{
    assert(saturator);
    switch (saturator->curve)
    {	
	case SINUSOIDAL:
		*input = px_saturate_sinusoidal(*input);
		break;

	case TANGENT:
		*input = px_saturate_tangent(*input);	
    		break;
    }
}

static void px_stereo_saturator_process(px_saturator* saturator, float* input_left, float* input_right)
{
    assert(saturator);
    switch (saturator->curve)
    {	
	case SINUSOIDAL:
		*input_left = px_saturate_sinusoidal(*input_left);
		*input_right = px_saturate_sinusoidal(*input_right);
		break;

	case TANGENT:
		*input_left = px_saturate_tangent(*input_left);	
    		*input_right = px_saturate_tangent(*input_right);
		break;
    }
}

static inline float px_saturate_sinusoidal(float input)
{
    float output = sin(input);
    return output;
}

static inline float px_saturate_tangent(float input)
{
    float output = tanh(input);
    return output;
}
