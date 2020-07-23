#ifndef OPENAL_AUDIO_MANAGER_HEADER
#define OPENAL_AUDIO_MANAGER_HEADER

#include <OpenAL/al.h>

typedef struct
{
	ALenum format;
	char* data;
	int size_bytes;
	int freq;
}AudioSource;

ALuint get_debug_sound();

void set_looping_sound(ALuint audio, int state);
void play_sound(ALuint audio);

void setup_openal_audio();


#endif