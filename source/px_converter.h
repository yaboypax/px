#include "px_buffer.h"

#ifndef PX_CONVERTER_H
#define PX_CONVERTER_H


/*

	Convert file into px_buffer float buffer.
	So far only .wav is supported.

	USE:

	px_buffer buffer;
	px_convert(&buffer, file_path);
*/


typedef struct {
	char    riff[4]; // buffer for reading RIFF Header info
	int32_t file_size;
	int32_t	format_length;
	int16_t	format_type;
	int16_t	channels;
	int32_t	sample_rate;
	int32_t	bytes_per_second;
	int16_t	block_align;
	int16_t	bits_per_sample;
	int32_t	data_size;
} px_wav_data;

static void px_convert(px_buffer* buffer, const char* path);
static bool px_convert_wav(px_buffer* buffer, const char* path);


static void px_convert(px_buffer* buffer, const char* path)
{
	assert(buffer);
	char* extension = strrchr(path,'.');	
	if(extension != NULL ) {
     		if(strcmp(extension,".wav") == 0) {
			bool result = px_convert_wav(buffer, path);
			if (!result) printf("Error Reading Wave File At: %s\n", path);
			return;
      		} else {
			printf("File Format Unsupported.\n Supported Formats: .wav\n");
		}
  	}	 
}

static bool px_convert_wav(px_buffer* buffer, const char* path) 
{

	px_wav_data data = {0};
	FILE* file = fopen(path, "rb");
	if (file==NULL) {
		printf("File Open (fopen) failure at - %s\n", path);
		return false;
	} else {
		printf("Opening File - %s\n", path);
	}

	fread(data.riff,1,4, file);	
	printf("RIFF Header: %s\n", data.riff);
	if (data.riff[0]!='R' || data.riff[1] != 'I' || data.riff[2] != 'F' || data.riff[3] != 'F') {
		printf("RIFF failed\n");
		fclose(file);
		return false;
	}

	fread(&data.file_size, 4, 1, file);
	printf("File Size: %d\n", data.file_size);
	

	fread(data.riff,1,4,file);
	printf("WAVE Header: %s\n", data.riff);
	if (data.riff[0]!='W' || data.riff[1] != 'A' || data.riff[2] != 'V' || data.riff[3] != 'E') {
		printf("WAVE failed\n");
		fclose(file);
		return false;
	}

	
	fread(data.riff,1,4, file);
	printf("Format: %s\n", data.riff);
	if (data.riff[0]!='f' || data.riff[1] != 'm' || data.riff[2] != 't' || data.riff[3] != ' ') { 
		printf("fmt failed\n");
		fclose(file);
		return false;
	}
	
	fread(&data.format_length, 4, 1, file);
	printf("Format Length: %d\n", data.format_length);

	fread(&data.format_type, 2, 1, file);
	printf("Format Type: %d\n", data.format_type);

	fread(&data.channels, 2, 1, file);
	printf("Number of Channels: %d\n", data.channels);

	fread(&data.sample_rate, 4, 1, file);
	printf("Sample Rate: %d\n", data.sample_rate);

	fread(&data.bytes_per_second, 4, 1, file);
	printf("Bytes Per Second: %d\n", data.bytes_per_second);

	fread(&data.block_align, 2, 1, file);
	printf("Block Align: %d\n", data.block_align);

	fread(&data.bits_per_sample, 2, 1, file);
	printf("Bits Per Sample: %d\n", data.bits_per_sample);

	fread(data.riff, 1, 4, file);
	if(data.riff[0] != 'd' || data.riff[1] != 'a' || data.riff[2] != 't' || data.riff[3] != 'a') {
		printf("data failed\n");
		fclose(file);
		return false;
	}
	
	fread(&data.data_size, 4, 1, file);
	printf("Data Size: %d\n", data.data_size);
	
	px_buffer_initialize(buffer, data.channels, (data.data_size/data.channels) );
	fclose(file);	
	return true;
}

#endif
