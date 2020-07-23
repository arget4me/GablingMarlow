#ifndef OPENAL_AUDIO_MANAGER_HEADER
#define OPENAL_AUDIO_MANAGER_HEADER

#include <OpenAL/al.h>

struct AudioSource
{
	ALenum format;
	char* data;
	int size_bytes;
	int freq;
};

ALuint get_debug_sound();

void play_sound(ALuint audio);

void setup_openal_audio();


#endif