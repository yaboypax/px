#include "px_globals.h"
#include "px_buffer.h"

#ifndef PX_DELAY_H
#define PX_DELAY_H

typedef struct
{
    float seconds;

    // split for sample interpolation
    float fraction;
    int whole;
} delay_time;

typedef struct
{
   float sample_rate;
   float feedback;
   delay_time time;
   float max_time; // in seconds
   float dry_wet;
} px_delay_parameters;

typedef struct
{
    px_delay_parameters parameters;
    px_circular_buffer buffer;

} px_delay_line;


typedef struct
{
    px_delay_line left;
    px_delay_line right;
} px_stereo_delay;

static void px_delay_mono_initialize(px_delay_line* delay, float sample_rate, float max_time);
static void px_delay_mono_prepare(px_delay_line* delay, float sample_rate);
static void px_delay_mono_set_time(px_delay_line* delay, float time);
static void px_delay_mono_set_feedback(px_delay_line* delay, float feedback);
static void px_delay_mono_process(px_delay_line* delay, float* input);

static void px_delay_stereo_initialize(px_stereo_delay* delay, float sample_rate, float max_time);
static void px_delay_stereo_prepare(px_stereo_delay* delay, float sample_rate);
static void px_delay_stereo_set_time(px_stereo_delay* delay, float time, CHANNEL_FLAG channel);
static void px_delay_stereo_set_feedback(px_stereo_delay* delay, float feedback, CHANNEL_FLAG channel);
static void px_delay_stereo_process(px_stereo_delay* delay, float* input_left, float* input_right);

static void px_delay_mono_initialize(px_delay_line* delay, float sample_rate, float max_time)
{
   assert(delay);
  
   // TODO REMOVE THIS 
   if (delay->parameters.max_time == max_time)
   {
		px_delay_mono_prepare(delay, sample_rate);
		return;
   }

   delay_time time = { 1.f, 0.f, 1 };
   px_delay_parameters parameters = {  sample_rate, 0.5f, time, max_time, 0.5f };
   delay->parameters = parameters;

   int max_samples = sample_rate * max_time;
   px_circular_initialize(&delay->buffer, max_samples);
}

static void px_delay_stereo_initialize(px_stereo_delay* delay, float sample_rate, float max_time)
{
	assert(delay);

	delay_time time = {1.f, 0.f, 1 };
	px_delay_parameters parameters = { sample_rate, 0.5f, time, max_time, 0.5f };
	
	delay->left.parameters = parameters;
	delay->right.parameters = parameters;

	int max_samples = sample_rate * max_time;
	px_circular_initialize(&delay->left.buffer, max_samples);
	px_circular_initialize(&delay->right.buffer, max_samples);	
}

static void px_delay_mono_prepare(px_delay_line* delay, float sample_rate)
{
    assert(delay);
	
	delay->parameters.sample_rate = sample_rate;
	int max_samples = sample_rate * delay->parameters.max_time;
	px_circular_initialize(&delay->buffer, max_samples);
}

static void px_delay_stereo_prepare(px_stereo_delay* delay, float sample_rate)
{
	assert(delay);
	
	delay->left.parameters.sample_rate = sample_rate;
	delay->right.parameters.sample_rate = sample_rate;

	//error with initialization
	assert(delay->left.parameters.max_time == delay->right.parameters.max_time);
	int max_samples = sample_rate * delay->left.parameters.max_time;

	px_circular_initialize(&delay->left.buffer, max_samples);
	px_circular_initialize(&delay->right.buffer, max_samples);
}

static void px_delay_mono_set_time(px_delay_line* delay, float time)
{
   assert(delay);
   assert(time > 0 && time < delay->parameters.max_time);
   delay->parameters.time.seconds = time;
   
   float time_in_samples = delay->parameters.sample_rate * time;
   delay->parameters.time.whole = (int)floorf(time_in_samples);
   delay->parameters.time.fraction = time_in_samples - delay->parameters.time.whole;
}

static void px_delay_stereo_set_time(px_stereo_delay* delay, float time, CHANNEL_FLAG channel)
{
	assert(delay);
	assert(time>0 && time < delay->left.parameters.max_time);
	switch (channel)
	{
			case BOTH:
			{
				px_delay_mono_set_time(&delay->left, time);
				px_delay_mono_set_time(&delay->right, time);
			}
			case LEFT:
			{
				px_delay_mono_set_time(&delay->left, time);
			}	
			case RIGHT:
			{
				px_delay_mono_set_time(&delay->right, time);
			}
	}	

}

static void px_delay_mono_set_feedback(px_delay_line* delay, float feedback)
{
   assert(delay);
   assert(feedback < 1.01f);

   delay->parameters.feedback = feedback;
}

static void px_delay_stereo_set_feedback(px_stereo_delay* delay, float feedback, CHANNEL_FLAG channel)
{
	assert(delay);
	assert(feedback < 1.01f);

	switch (channel)
	{
			case BOTH:
			{
				px_delay_mono_set_feedback(&delay->left, feedback);
				px_delay_mono_set_feedback(&delay->right, feedback);
			}
			case LEFT:
			{
				px_delay_mono_set_feedback(&delay->left, feedback);
			}	
			case RIGHT:
			{
				px_delay_mono_set_feedback(&delay->right, feedback);
			}
	}	
}

static void px_delay_mono_process(px_delay_line* delay, float* input)
{
    px_assert(delay, input);

    int read1 = (delay->buffer.head - delay->parameters.time.whole + delay->buffer.max_length) % delay->buffer.max_length;
    int read2 = (read1 + 1) % delay->buffer.max_length;

    float delayed1 = px_circular_get_sample(&delay->buffer, (size_t) read1);
    float delayed2 = px_circular_get_sample(&delay->buffer, (size_t) read2);
    
    // linear interpolation
    float delayed_interp = delayed1 + delay->parameters.time.fraction * (delayed2 - delayed1);
    
    float feedback = (*input) + (delay->parameters.feedback * delayed_interp);
    px_circular_push(&delay->buffer, feedback);

    float output = ((1.0f - delay->parameters.dry_wet) * (*input)) + (delay->parameters.dry_wet * delayed_interp);
    *input = output;
}

static void px_delay_stereo_process(px_stereo_delay* delay, float* input_left, float* input_right)
{
	px_assert(delay, input_left, input_right);

	px_delay_mono_process(&delay->left, input_left);
	px_delay_mono_process(&delay->right, input_right);

}

#endif
