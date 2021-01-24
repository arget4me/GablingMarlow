#ifndef WAV_LOADER_HEADER
#define WAV_LOADER_HEADER

typedef struct
{
	char* data;
	int freq;
	int bits_per_sample;
	short num_channels;
	int size_bytes;
}Wav_file;

Wav_file load_wav(char* buffer, int buffersize);

#endif

