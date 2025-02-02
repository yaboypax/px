#include <stdarg.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>



#ifndef PX_GLOBALS_H
#define PX_GLOBALS_H

// Math Constants
// ---------------------------------
#define DC_OFFSET 1.0E-25
#define PI 3.141592653589793
#define sgn(val) ((0 < val) - (val < 0))
// ---------------------------------
// Gain

// linear -> dB conversion
static inline float lin2dB(float lin) {
    static const float LOG_2_DB = 8.6858896380650365530225783783321;	// 20 / ln( 10 )
    return log(lin) * LOG_2_DB;
}

// dB -> linear conversion
static inline float dB2lin(float dB) {
    static const float DB_2_LOG = 0.11512925464970228420089957273422;	// ln( 10 ) / 20
    return exp(dB * DB_2_LOG);
}
// -------------------------------------
//
// Mid Side
//
//

	typedef enum
	{
		BOTH = 0,
		LEFT,
		RIGHT,
		MID,
		SIDE
	} CHANNEL_FLAG;


typedef struct
{
    float left;
    float right;
} px_ms_decoded;

typedef struct
{
    float mid;
    float side;
} px_ms_encoded;


static inline px_ms_encoded px_ms_encode(px_ms_decoded decoded)
{
	px_ms_encoded encoded;
	encoded.mid = 0.5f * (decoded.left + decoded.right);
	encoded.side = 0.5f * (decoded.left - decoded.right);
	return encoded;
}

static inline px_ms_decoded px_ms_decode(px_ms_encoded encoded)
{
	px_ms_decoded decoded;
	decoded.left = encoded.mid + encoded.side;
	decoded.right = encoded.mid - encoded.side;
	return decoded;
}

// assert for process functions
// ------------------------------------------------------------------------------------------------------


	
static void px_assert_mono(void* control_pointer, float* value_pointer)
{
	// check if DSP object passed is NULL or uninitialized
	assert(control_pointer);

	// check if value is valid float pointer
	assert(value_pointer);
}

static void px_assert_stereo(void* control_pointer, float* channel_one_pointer, float* channel_two_pointer)
{
	// check if DSP object passed is NULL or uninitialized
	assert(control_pointer);

	// check if value is valid float pointer
	assert(channel_one_pointer);
	assert(channel_two_pointer);
}

#define EXPAND(x) x
#define GET_ASSERT_MACRO(_1, _2, _3, NAME, ...) NAME

#define px_assert(...) EXPAND(GET_ASSERT_MACRO(__VA_ARGS__, px_assert_stereo, px_assert_mono)(__VA_ARGS__))
#define px_map(value, inputMin, inputMax, outputMin, outputMax) \
    ((inputMin > inputMax) ? (outputMin) : \
    ((outputMin) + ((outputMax) - (outputMin)) * (((float)(value) - (float)(inputMin)) / ((float)(inputMax) - (float)(inputMin)))))

#endif 
