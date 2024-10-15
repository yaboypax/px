#ifdef __cplusplus
extern "C"
{
#endif

#include "px_biquad.h"
#include "px_vector.h"
#include "px_globals.h"


#define MAX_BANDS 24

	typedef enum
	{
		BOTH = 0,
		LEFT,
		RIGHT,
		MID,
		SIDE
	} CHANNEL_FLAG;

	typedef struct px_mono_equalizer
	{
		px_vector filter_bank;  // type-generic vector filled with px_mono_biquad filters
		float sample_rate;
		int num_bands;
	} px_mono_equalizer;

	typedef struct px_stereo_equalizer
	{
		px_mono_equalizer left;
		px_mono_equalizer right;
	} px_stereo_equalizer;

	typedef struct px_ms_equalizer
	{
		px_mono_equalizer mid;
		px_mono_equalizer side;
	} px_ms_equalizer;

	// ----------------------------------------------------------------------------------------------------

#ifdef PX_USE_GENERIC

#define px_equalizer_process(a,b,...) _Generic((a),		\
	px_mono_equalizer*: px_equalizer_mono_process,		\
	px_stereo_equalizer*: px_equalizer_stereo_process,	\
	px_ms_equalizer*: px_equalizer_ms_process)		\
		(a,b,__VA_ARGS__)					\

#define px_equalizer_initialize(a,b) _Generic((a),		\
	px_mono_equalizer*: px_equalizer_mono_initialize,	\
	px_stereo_equalizer*: px_equalizer_stereo_initialize,	\
	px_ms_equalizer*: px_equalizer_ms_initialize)		\
		(a,b)

#define px_equalizer_add_band(a,b,c,d,e) _Generic((a),		\
	px_mono_equalizer*: px_equalizer_mono_add_band,		\
	px_stereo_equalizer*: px_equalizer_stereo_add_band,	\
	px_ms_equalizer*: px_equalizer_ms_add_band)		\
		(a,b,c,d,e)					\

#define px_equalizer_remove_band(a,b) _Generic((a),		\
	px_mono_equalizer*: px_equalizer_mono_remove_band,	\
	px_stereo_equalizer*: px_equalizer_stereo_remove_band,	\
	px_ms_equalizer*: px_equalizer_ms_remove_band)		\
		(a,b)


#define px_equalizer_set_frequency(a,b,c,...) _Generic((a),		\
	px_mono_equalizer*: px_equalizer_mono_set_frequency,		\
	px_stereo_equalizer*: px_equalizer_stereo_set_frequency,	\
	px_ms_equalizer*: px_equalizer_ms_set_frequency)		\
		(a,b,c,__VA_ARGS__)

#define px_equalizer_set_quality(a,b,c,...) _Generic((a),		\
	px_mono_equalizer*: px_equalizer_mono_set_quality,		\
	px_stereo_equalizer*: px_equalizer_stereo_set_quality,		\
	px_ms_equalizer*: px_equalizer_ms_set_quality)		\
		(a,b,c,__VA_ARGS__)

#define px_equalizer_set_gain(a,b,c,...) _Generic((a),			\
	px_mono_equalizer*: px_equalizer_mono_set_gain,		\
	px_stereo_equalizer*: px_equalizer_stereo_set_gain,		\
	px_ms_equalizer*: px_equalizer_ms_set_quality)		\
		(a,b,c,__VA_ARGS__)

#define px_equalizer_set_type(a,b,c,...) _Generic((a),			\
	px_mono_equalizer*: px_equalizer_mono_set_type,		\
	px_stereo_equalizer*: px_equalizer_stereo_set_type,		\
	px_ms_equalizer*: px_equalizer_ms_set_type)			\
		(a,b,c,__VA_ARGS__)
#endif

// mono
	static void px_equalizer_mono_process(px_mono_equalizer* equalizer, float* input);
	static void px_equalizer_mono_initialize(px_mono_equalizer* equalizer, float sample_rate);
	static void px_equalizer_mono_add_band(px_mono_equalizer* equalizer, float frequency, float quality, float gain, BIQUAD_FILTER_TYPE type);
	static void px_equalizer_mono_remove_band(px_mono_equalizer* equalizer, size_t index);

	static void px_equalizer_mono_set_frequency(px_mono_equalizer* equalizer, size_t index, float in_frequency);
	static void px_equalizer_mono_set_quality(px_mono_equalizer* equalizer, size_t index, float in_quality);
	static void px_equalizer_mono_set_gain(px_mono_equalizer* equalizer, size_t index, float in_gain);
	static void px_equalizer_mono_set_type(px_mono_equalizer* equalizer, size_t index, BIQUAD_FILTER_TYPE in_type);

	// stereo
	static void px_equalizer_stereo_process(px_stereo_equalizer* stereo_equalizer, float* input_left, float* input_right);
	static void px_equalizer_stereo_initialize(px_stereo_equalizer* stereo_equalizer, float sample_rate);
	static void px_equalizer_stereo_add_band(px_stereo_equalizer* stereo_equalizer, float frequency, float quality, float gain, BIQUAD_FILTER_TYPE type);
	static void px_equalizer_stereo_remove_band(px_stereo_equalizer* stereo_equalizer, size_t index);

	static void px_equalizer_stereo_set_frequency(px_stereo_equalizer* stereo_equalizer, size_t index, float in_frequency, CHANNEL_FLAG channel);
	static void px_equalizer_stereo_set_quality(px_stereo_equalizer* stereo_equalizer, size_t index, float in_quality, CHANNEL_FLAG channel);
	static void px_equalizer_stereo_set_gain(px_stereo_equalizer* stereo_equalizer, size_t index, float in_gain, CHANNEL_FLAG channel);
	static void px_equalizer_stereo_set_type(px_stereo_equalizer* stereo_equalizer, size_t index, BIQUAD_FILTER_TYPE in_type, CHANNEL_FLAG channel);

	// mid/side
	static void px_equalizer_ms_process(px_ms_equalizer* ms_equalizer, float* input_left, float* input_right);
	static px_ms_encoded px_equalizer_ms_process_and_return(px_ms_equalizer* ms_equalizer, float input_left, float input_right);
	static void px_equalizer_ms_initialize(px_ms_equalizer* ms_equalizer, float sample_rate);
	static void px_equalizer_ms_add_band(px_ms_equalizer* ms_equalizer, float frequency, float quality, float gain, BIQUAD_FILTER_TYPE type);
	static void px_equalizer_ms_remove_band(px_ms_equalizer* ms_equalizer, size_t index);

	static void px_equalizer_ms_set_frequency(px_ms_equalizer* ms_equalizer, size_t index, float in_frequency, CHANNEL_FLAG channel);
	static void px_equalizer_ms_set_quality(px_ms_equalizer* ms_equalizer, size_t index, float in_quality, CHANNEL_FLAG channel);
	static void px_equalizer_ms_set_gain(px_ms_equalizer* ms_equalizer, size_t index, float in_gain, CHANNEL_FLAG channel);
	static void px_equalizer_ms_set_type(px_ms_equalizer* ms_equalizer, size_t index, BIQUAD_FILTER_TYPE in_type, CHANNEL_FLAG channel);


	// ----------------------------------------------------------------------------------------------------

	static void px_equalizer_mono_process(px_mono_equalizer* equalizer, float* input)
	{
		px_assert(equalizer, input);
		for (int i = 0; i < equalizer->num_bands; ++i)
		{
			px_biquad_process((px_biquad*)px_vector_get(&equalizer->filter_bank, i), input);
		}
	}

	static void px_equalizer_stereo_process(px_stereo_equalizer* stereo_equalizer, float* input_left, float* input_right)
	{
		px_assert(stereo_equalizer, input_left, input_right);
		px_equalizer_mono_process(&stereo_equalizer->left, input_left);
		px_equalizer_mono_process(&stereo_equalizer->right, input_right);

	}

	static void px_equalizer_ms_process(px_ms_equalizer* ms_equalizer, float* input_left, float* input_right)
	{
		px_assert(ms_equalizer, input_left, input_right);
		px_ms_decoded decoded = { 0.f, 0.f };

		decoded.left = *input_left;
		decoded.right = *input_right;

		px_ms_encoded encoded = px_ms_encode(decoded);

		px_equalizer_mono_process(&ms_equalizer->mid, &encoded.mid);
		px_equalizer_mono_process(&ms_equalizer->side, &encoded.side);

		decoded = px_ms_decode(encoded);

		*input_left = decoded.left;
		*input_right = decoded.right;
	}
	
	static px_ms_encoded px_equalizer_ms_process_and_return(px_ms_equalizer* ms_equalizer, float input_left, float input_right)
	{
		assert(ms_equalizer);
		px_ms_decoded decoded = { 0.f, 0.f };

		decoded.left = input_left;
		decoded.right = input_right;

		px_ms_encoded encoded = px_ms_encode(decoded);

		px_equalizer_mono_process(&ms_equalizer->mid, &encoded.mid);
		px_equalizer_mono_process(&ms_equalizer->side, &encoded.side);

		return encoded;
	}

	static void px_equalizer_mono_initialize(px_mono_equalizer* equalizer, float sample_rate)
	{
		assert(equalizer);
		equalizer->sample_rate = sample_rate;
		equalizer->num_bands = 0;

		px_vector_initialize(&equalizer->filter_bank);
	}

	static void px_equalizer_stereo_initialize(px_stereo_equalizer* stereo_equalizer, float sample_rate)
	{
		assert(stereo_equalizer);
		px_equalizer_mono_initialize(&stereo_equalizer->left, sample_rate);
		px_equalizer_mono_initialize(&stereo_equalizer->right, sample_rate);
	}

	static void px_equalizer_ms_initialize(px_ms_equalizer* ms_equalizer, float sample_rate)
	{
		assert(ms_equalizer);
		px_equalizer_mono_initialize(&ms_equalizer->mid, sample_rate);
		px_equalizer_mono_initialize(&ms_equalizer->side, sample_rate);
	}


	static void px_equalizer_mono_add_band(px_mono_equalizer* equalizer, float frequency, float quality, float gain, BIQUAD_FILTER_TYPE type)
	{
		assert(equalizer);
		px_biquad* new_filter = px_biquad_create(equalizer->sample_rate, type);

		px_biquad_set_frequency(new_filter, frequency);
		px_biquad_set_quality(new_filter, quality);
		px_biquad_set_gain(new_filter, gain);

		px_vector_push(&equalizer->filter_bank, new_filter);
		equalizer->num_bands++;
	}

	static void px_equalizer_stereo_add_band(px_stereo_equalizer* stereo_equalizer, float frequency, float quality, float gain, BIQUAD_FILTER_TYPE type)
	{
		assert(stereo_equalizer);
		px_equalizer_mono_add_band(&stereo_equalizer->left, frequency, quality, gain, type);
		px_equalizer_mono_add_band(&stereo_equalizer->right, frequency, quality, gain, type);
	}

	static void px_equalizer_ms_add_band(px_ms_equalizer* ms_equalizer, float frequency, float quality, float gain, BIQUAD_FILTER_TYPE type)
	{
		assert(ms_equalizer);
		px_equalizer_mono_add_band(&ms_equalizer->mid, frequency, quality, gain, type);
		px_equalizer_mono_add_band(&ms_equalizer->side, frequency, quality, gain, type);
	}


	static void px_equalizer_mono_remove_band(px_mono_equalizer* equalizer, size_t index)
	{
		if (index < equalizer->num_bands)
		{
			px_biquad_destroy((px_biquad*)px_vector_get(&equalizer->filter_bank, index));
			px_vector_remove(&equalizer->filter_bank, index);
			equalizer->num_bands--;
		}
	}

	static void px_equalizer_stereo_remove_band(px_stereo_equalizer* stereo_equalizer, size_t index)
	{
		assert(stereo_equalizer);
		px_equalizer_mono_remove_band(&stereo_equalizer->left, index);
		px_equalizer_mono_remove_band(&stereo_equalizer->right, index);
	}


	static void px_equalizer_ms_remove_band(px_ms_equalizer* ms_equalizer, size_t index)
	{
		assert(ms_equalizer);
		px_equalizer_mono_remove_band(&ms_equalizer->mid, index);
		px_equalizer_mono_remove_band(&ms_equalizer->side, index);
	}


	static void px_equalizer_mono_set_frequency(px_mono_equalizer* equalizer, size_t index, float in_frequency)
	{
		assert(equalizer);
		if (index < equalizer->num_bands)
		{
			px_biquad* filter = (px_biquad*)px_vector_get(&equalizer->filter_bank, index);
			px_biquad_set_frequency(filter, in_frequency);
		}
	}

	static void px_equalizer_stereo_set_frequency(px_stereo_equalizer* stereo_equalizer, size_t index, float in_frequency, CHANNEL_FLAG channel)
	{
		assert(stereo_equalizer);
		switch (channel)
		{
		case BOTH:
		{
			px_equalizer_mono_set_frequency(&stereo_equalizer->left, index, in_frequency);
			px_equalizer_mono_set_frequency(&stereo_equalizer->right, index, in_frequency);
		}
		case LEFT:
		{
			px_equalizer_mono_set_frequency(&stereo_equalizer->left, index, in_frequency);
		}
		case RIGHT:
		{
			px_equalizer_mono_set_frequency(&stereo_equalizer->right, index, in_frequency);
		}
		}
	}

	static void px_equalizer_ms_set_frequency(px_ms_equalizer* ms_equalizer, size_t index, float in_frequency, CHANNEL_FLAG channel)
	{
		assert(ms_equalizer);
		switch (channel)
		{
		case BOTH:
		{
			px_equalizer_mono_set_frequency(&ms_equalizer->mid, index, in_frequency);
			px_equalizer_mono_set_frequency(&ms_equalizer->side, index, in_frequency);
		}
		case LEFT:
		{
			px_equalizer_mono_set_frequency(&ms_equalizer->mid, index, in_frequency);
		}
		case RIGHT:
		{
			px_equalizer_mono_set_frequency(&ms_equalizer->side, index, in_frequency);
		}
		}
	}


	static void px_equalizer_mono_set_quality(px_mono_equalizer* equalizer, size_t index, float in_quality)
	{
		assert(equalizer);
		if (index < equalizer->num_bands)
		{
			px_biquad* filter = (px_biquad*)px_vector_get(&equalizer->filter_bank, index);
			px_biquad_set_quality(filter, in_quality);
		}
	}

	static void px_equalizer_stereo_set_quality(px_stereo_equalizer* stereo_equalizer, size_t index, float in_quality, CHANNEL_FLAG channel)
	{
		assert(stereo_equalizer);
		switch (channel)
		{
		case BOTH:
		{
			px_equalizer_mono_set_quality(&stereo_equalizer->left, index, in_quality);
			px_equalizer_mono_set_quality(&stereo_equalizer->right, index, in_quality);
		}
		case LEFT:
		{
			px_equalizer_mono_set_quality(&stereo_equalizer->left, index, in_quality);
		}
		case RIGHT:
		{
			px_equalizer_mono_set_quality(&stereo_equalizer->right, index, in_quality);
		}
		}
	}

	static void px_equalizer_ms_set_quality(px_ms_equalizer* ms_equalizer, size_t index, float in_quality, CHANNEL_FLAG channel)
	{
		assert(ms_equalizer);
		switch (channel)
		{
		case BOTH:
		{
			px_equalizer_mono_set_quality(&ms_equalizer->mid, index, in_quality);
			px_equalizer_mono_set_quality(&ms_equalizer->side, index, in_quality);
		}
		case MID:
		{
			px_equalizer_mono_set_quality(&ms_equalizer->mid, index, in_quality);
		}
		case SIDE:
		{
			px_equalizer_mono_set_quality(&ms_equalizer->side, index, in_quality);
		}
		}
	}


	static void px_equalizer_mono_set_gain(px_mono_equalizer* equalizer, size_t index, float in_gain)
	{
		assert(equalizer);
		if (index < equalizer->num_bands)
		{
			px_biquad* filter = (px_biquad*)px_vector_get(&equalizer->filter_bank, index);
			px_biquad_set_gain(filter, in_gain);
		}
	}

	static void px_equalizer_stereo_set_gain(px_stereo_equalizer* stereo_equalizer, size_t index, float in_gain, CHANNEL_FLAG channel)
	{
		assert(stereo_equalizer);
		switch (channel)
		{
		case BOTH:
		{
			px_equalizer_mono_set_gain(&stereo_equalizer->left, index, in_gain);
			px_equalizer_mono_set_gain(&stereo_equalizer->right, index, in_gain);
		}
		case LEFT:
		{
			px_equalizer_mono_set_gain(&stereo_equalizer->left, index, in_gain);
		}
		case RIGHT:
		{
			px_equalizer_mono_set_gain(&stereo_equalizer->right, index, in_gain);
		}
		}
	}

	static void px_equalizer_ms_set_gain(px_ms_equalizer* ms_equalizer, size_t index, float in_gain, CHANNEL_FLAG channel)
	{
		assert(ms_equalizer);
		switch (channel)
		{
		case BOTH:
		{
			px_equalizer_mono_set_gain(&ms_equalizer->mid, index, in_gain);
			px_equalizer_mono_set_gain(&ms_equalizer->side, index, in_gain);
		}
		case MID:
		{
			px_equalizer_mono_set_gain(&ms_equalizer->mid, index, in_gain);
		}
		case SIDE:
		{
			px_equalizer_mono_set_gain(&ms_equalizer->side, index, in_gain);
		}
		}
	}

	static void px_equalizer_mono_set_type(px_mono_equalizer* equalizer, size_t index, BIQUAD_FILTER_TYPE in_type)
	{
		assert(equalizer);
		if (index < equalizer->num_bands)
		{
			px_biquad* filter = (px_biquad*)px_vector_get(&equalizer->filter_bank, index);
			px_biquad_set_type(filter, in_type);
		}
	}

	static void px_equalizer_stereo_set_type(px_stereo_equalizer* stereo_equalizer, size_t index, BIQUAD_FILTER_TYPE in_type, CHANNEL_FLAG channel)
	{
		assert(stereo_equalizer);
		switch (channel)
		{
		case BOTH:
		{
			px_equalizer_mono_set_type(&stereo_equalizer->left, index, in_type);
			px_equalizer_mono_set_type(&stereo_equalizer->right, index, in_type);
		}
		case LEFT:
		{
			px_equalizer_mono_set_type(&stereo_equalizer->left, index, in_type);
		}
		case RIGHT:
		{
			px_equalizer_mono_set_type(&stereo_equalizer->right, index, in_type);
		}
		}
	}

	static void px_equalizer_ms_set_type(px_ms_equalizer* ms_equalizer, size_t index, BIQUAD_FILTER_TYPE in_type, CHANNEL_FLAG channel)
	{
		assert(ms_equalizer);
		switch (channel)
		{
		case BOTH:
		{
			px_equalizer_mono_set_type(&ms_equalizer->mid, index, in_type);
			px_equalizer_mono_set_type(&ms_equalizer->side, index, in_type);
		}
		case MID:
		{
			px_equalizer_mono_set_type(&ms_equalizer->mid, index, in_type);
		}
		case SIDE:
		{
			px_equalizer_mono_set_type(&ms_equalizer->side, index, in_type);
		}
		}
	}

#ifdef __cplusplus
}
#endif
