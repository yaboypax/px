#include <stdlib.h>
#include <assert.h>
#include "px_globals.h"

typedef enum
{
    HARD,
    SOFT,	// quintic 
    SMOOTH	// arctangent
} CLIP_TYPE;

typedef struct
{
   CLIP_TYPE type;
} px_clipper;

// -----------------------------------------------------------------------------------------
// api
static px_clipper* px_clipper_create();
static void px_clipper_destroy(px_clipper* clipper);

static void px_clipper_initialize(px_clipper* clipper);
static void px_clipper_set_type(px_clipper* clipper, CLIP_TYPE in_type);

static void px_clipper_mono_process(px_clipper* clipper, float* input);
static void px_clipper_stereo_process(px_clipper* clipper, float* input_left, float* input_right);

// ---------------------------------------------------------------------------------------------
// inline functions
static inline float hard_clip(float input);
static inline float quintic_clip(float input);
static inline float arctangent_clip(float input);
// ---------------------------------------------------------------------------------------------

static px_clipper* px_clipper_create()
{
    px_clipper* clipper = (px_clipper*)malloc(sizeof(px_clipper));
	px_clipper_initialize(clipper);
    return clipper;
}

static void px_clipper_destroy(px_clipper* clipper)
{
    if (clipper)
    	free(clipper);
}

static void px_clipper_initialize(px_clipper* clipper)
{
    assert(clipper);
    clipper->type = HARD;
}

static void px_clipper_set_type(px_clipper* clipper, CLIP_TYPE in_type)
{
    assert(clipper);
    clipper->type = in_type;    
}

static void px_clipper_mono_process(px_clipper* clipper, float* input)
{
	assert(clipper);
	printf("%d", clipper->type);
	switch (clipper->type)
	{
	case HARD:
	{
		*input = hard_clip(*input);
		break;
	}
	case SOFT:
	{
		*input = quintic_clip(*input);
		break;
	}
	case SMOOTH:
	{
		*input = arctangent_clip(*input);
		break;
	}
	default:
	{
		printf("clipper uninitialized");
		break;
	}
	}
}

static void px_clipper_stereo_process(px_clipper* clipper, float* input_left, float* input_right)
{
	assert(clipper);
	printf("%d", clipper->type);
	switch (clipper->type)
	{
	case HARD:
	{
		*input_left = hard_clip(*input_left);
		*input_right = hard_clip(*input_right);
		break;
	}
	case SOFT:
	{
		*input_left = quintic_clip(*input_left);
		*input_right = quintic_clip(*input_right);
		break;
	}
	case SMOOTH:
	{
		*input_left = arctangent_clip(*input_left);
		*input_right = arctangent_clip(*input_right);
		break;
	}
	default:
	{
		printf("clipper uninitialized");
		break;
	}
	}
}

static inline float hard_clip(float input)
{
	return sgn(input) * fmin(fabs(input), 1.0f);
}

static inline float quintic_clip(float input)
{
	if (fabs(input) < 1.25f)
		return input - (256.0f / 3125.0f) * powf(input, 5.0f);
	else
		return sgn(input) * 1.0f;
}

static inline float arctangent_clip(float input)
{
	return (2.0f / PI) * atan((1.6f * 0.6f) * input);
}



