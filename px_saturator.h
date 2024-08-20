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


static void px_saturator_initialize(px_saturator* saturator, SATURATION_CURVE curve);
static px_saturator* px_saturator_create(SATURATION_CURVE curve);
static void px_saturator_destroy(px_saturator* saturator);

static void px_saturator_set_drive(px_saturator* saturator, float drive);
static void px_saturator_set_curve(px_saturator* saturator, SATURATION_CURVE curve);
static void px_saturator_mono_process(px_saturator* saturator, float* input);
static void px_saturator_stereo_process(px_saturator* saturator, float* input_left, float* input_right);

static inline float px_saturate_sinusoidal(float input, float drive);
static inline float px_saturate_tangent(float input, float drive);


static void px_saturator_initialize(px_saturator* saturator, SATURATION_CURVE curve)
{
    assert(saturator);
    saturator->drive = 0.f;
    saturator->curve = curve;
}

static px_saturator* px_saturator_create(SATURATION_CURVE curve)
{
    px_saturator* saturator = (px_saturator*)malloc(sizeof(px_saturator));
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

static void px_saturator_mono_process(px_saturator* saturator, float* input)
{
    assert(saturator);
    switch (saturator->curve)
    {	
	case SINUSOIDAL:
		*input = px_saturate_sinusoidal(*input, saturator->drive);
		break;

	case TANGENT:
		*input = px_saturate_tangent(*input, saturator->drive);	
    		break;
    }
}

static void px_saturator_stereo_process(px_saturator* saturator, float* input_left, float* input_right)
{
    assert(saturator);
    switch (saturator->curve)
    {	
	case SINUSOIDAL:
		*input_left = px_saturate_sinusoidal(*input_left, saturator->drive);
		*input_right = px_saturate_sinusoidal(*input_right, saturator->drive);
		break;

	case TANGENT:
		*input_left = px_saturate_tangent(*input_left, saturator->drive);
    		*input_right = px_saturate_tangent(*input_right, saturator->drive);
		break;
    }
}

static inline float px_saturate_sinusoidal(float input, float drive)
{
    float output = sin(input);
    return output;
}

static inline float px_saturate_tangent(float input, float drive)
{
    if (drive == 0.f)
    {
	return input
    }
    else
    {
    	float output = tanh(input * drive);
    	return output;
    }
}
