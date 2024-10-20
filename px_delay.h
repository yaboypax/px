
#ifndef PX_DELAY_H
#define PX_DELAY_H

#define PX_FLOAT_BUFFER 1
#include "px_globals.h"
#include "px_buffer.h"

typedef struct
{
   float sample_rate;
   float feedback;
   float time;
   float max_time;
   //int taps;
   float dry_wet;
} px_delay_parameters;

typedef struct
{
    px_delay_parameters parameters;
    px_circular_buffer buffer;

} px_delay_line;


typedef struct
{
    px_mono_delay left;
    px_mono_delay right;
    px_delay_parameters parameters;
} px_stereo_delay;


static void px_delay_mono_initialize(px_delay_line* delay, float sample_rate, float max_time)
{
   assert(delay);
   
   px_delay_parameters parameters = {  sample_rate, 0.5f, 1.f, max_time, 1.f };
   delay->parameters = parameters;

   int max_samples = sample_rate * max_time;
   px_circular_initialize(&delay->buffer, max_samples);
}

static void px_delay_mono_set_time(px_delay_line* delay, float time)
{
   assert(delay);
   assert(time < delay->parameters.max_time);

   delay->parameters.time = time;
}

static void px_delay_mono_set_feedback(px_delay_line* delay, float feedback)
{
   assert(delay);
   assert(feedback < 1.1f);

   delay->parameters.feedback = feedback;
}

static void px_delay_mono_process(px_delay_line* delay, float* input)
{
   px_assert(delay, input);

   px_circular_push(&delay->buffer, *input);
        
   int delay_samples = (int)(delay->parameters.sample_rate * delay->parameters.time);
   int read_pos = delay->buffer.head - delay_samples;
   if (read_pos < 0)
       read_pos += delay->buffer.max_length;

   float delayed = px_circular_get_sample(&delay->buffer, (size_t) read_pos);
   *input = *input + delay->parameters.feedback * delayed;  
}
/*
static inline float px_delay(px_buffer* buffer, float x, float feedback)
{
    for (int channel = 0; i < buffer->num_channels; ++i)
    {
    	for (int i = 0; i < buffer->num_samples; ++i)
    	{
    	    float y = px_buffer_get_sample(buffer, channel, i);
	    float d = x + feedback * y;
    	}
    }
}
*/

#endif
