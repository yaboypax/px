
#ifndef PX_DELAY_H
#define PX_DELAY_H

#include "px_globals.h"
#include "px_buffer.h"

typedef struct
{
   float time;
   int taps;
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


static void px_delay_mono_process(px_mono_delay* delay, float* input)
{
    
}

float inline px_delay(px_buffer* buffer, float x, float feedback)
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

#endif
