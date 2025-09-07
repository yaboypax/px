
#include "stdint.h"
#include "px_buffer.h"

#ifndef PX_CONVERTER_H
#define PX_CONVERTER_H
#define __STDC_WANT_LIB_EXT1__ 1

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

typedef struct {
	bool log_header;
	bool interleaved_buffer;
} px_converter_flags;


typedef enum { WAVE=0 } FILE_TYPE; 

static void px_convert(px_buffer* buffer, const char* path);
static bool px_convert_wav(px_buffer* buffer, const char* path);

static void px_write(px_buffer*, const char* path, FILE_TYPE type);
static bool px_write_wav(px_buffer*, const char* path, int32_t* sample_rate, int16_t* bit_depth, bool log_header);

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

static void px_write(px_buffer* buffer, const char* path, FILE_TYPE type)
{
	assert(buffer);
	int32_t sample_rate = 44100;
	int16_t bit_depth = 16;
	if (type == WAVE) {
		px_write_wav(buffer, path, &sample_rate, &bit_depth, true);
	} else printf("Only WAV Supported\n");
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
	
    // really frames or samples per channel
    int samples = (int)(data.data_size / (data.bits_per_sample / 8))/data.channels;
	px_buffer_initialize(buffer, data.channels, samples);
	printf("Buffer Samples: %d\n", samples);

	int16_t* raw_data = px_malloc(data.data_size); 
	fread(raw_data, 2, data.data_size, file);
	printf("Raw Data read without seg fault\n");

	for (size_t i = 0; i < samples; ++i)
	{
		px_buffer_set_sample(buffer, 0, i,(float)(raw_data[i]/INT16_MAX)); 
		px_buffer_set_sample(buffer, 1, i,(float)(raw_data[i+1]/INT16_MAX));
	}
	
	fclose(file);	
	return true;
}

static bool px_write_wav(px_buffer* buffer, const char* path, int32_t* sample_rate, int16_t* bit_depth, bool log_header)
{
	assert(buffer);

	FILE* file;
	errno_t err = fopen_s(&file, path, "wb");
	if (err != 0 ) {
		printf("Error opening file %s -> %d\n", path, err);
		return false;
	}

	const int32_t format_length = 16;
	const int16_t format_type = 1;
	const int     byte_count = *bit_depth/8;
	const int32_t bytes_per_second = (int32_t)(*sample_rate * byte_count * buffer->num_channels);
	const int16_t block_align = (int16_t)(buffer->num_channels * byte_count);
	const int32_t data_length = (int32_t)(buffer->num_samples * byte_count * buffer->num_channels);
	const int32_t file_size = data_length+36;

	if (log_header) printf("WRITE---------\nFormat Length: %d\nFormat Type: %d\nBytes Per Second: %d\nBlock Align: %d\nData Length: %d\nFile Size%d\n",
		   format_length, format_type, bytes_per_second, block_align, data_length, file_size);

	const char* riff = "RIFF";
	const char* wave = "WAVE";
	const char* fmt = "fmt ";
	const char* data = "data";

	fwrite(riff, 1, 4, file); //RIFF Header
	fwrite(&file_size, 4, 1, file);
	fwrite(wave, 1, 4, file);
	fwrite(fmt, 1, 4, file);
	
	fwrite(&format_length, 4, 1, file);
	fwrite(&format_type, 2, 1, file);
	fwrite(&buffer->num_channels, 2, 1, file);
	fwrite(sample_rate, 4, 1, file);
	
	fwrite(&bytes_per_second, 4, 1, file);
	fwrite(&block_align, 2, 1, file);
	fwrite(bit_depth, 2, 1, file);
	fwrite(data, 1, 4, file);
	fwrite(&data_length, 4, 1, file);
	
	for (int i = 0; i < buffer->num_samples; ++i)
	{
		int16_t left = (int16_t)(px_buffer_get_sample(buffer, 0, i)*INT16_MAX);
		int16_t right = (int16_t)(px_buffer_get_sample(buffer, 1, i)*INT16_MAX);

		fwrite(&left, 2, 1, file);
		fwrite(&right, 2, 1, file);
	}

	fclose(file);
	return true;
}
#endif
