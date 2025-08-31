#include "px_buffer.h"

/*

	Convert WAV file to px_buffer float buffer.

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

px_buffer  px_convert_stack(px_buffer* buffer, const char* path);
//px_buffer* px_convert_heap(const char* path);

bool px_convert_wav(px_buffer* buffer, const char* path);
bool px_convert_wav(px_buffer* buffer, const char* path) 
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
