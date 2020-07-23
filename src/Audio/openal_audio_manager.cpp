#include "openal_audio_manager.h"

#include "globals.h"
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include "wav_loader.h"
#include "Utils/readfile.h"
#include "Utils/logfile.h"

local_scope ALCdevice* Device;
local_scope ALCcontext* Context;
local_scope ALboolean g_bEAX;
local_scope ALuint g_Buffers;
local_scope ALuint source;
local_scope ALenum error;


#include <thread>         // std::this_thread::sleep_for
#include <chrono> 

ALuint get_debug_sound()
{
	return source;
}

void play_sound(ALuint audio)
{
	alSourcePlay(audio);
}

void setup_openal_audio()
{
	// Initialization
	Device = alcOpenDevice(nullptr); // select the "preferred device"

	if (Device) {
		Context = alcCreateContext(Device, nullptr);
		alcMakeContextCurrent(Context);
	}

	// Check for EAX 2.0 support
	g_bEAX = alIsExtensionPresent("EAX2.0");

	// Generate Buffers
	alGetError(); // clear error code 
	alGenBuffers(1, &g_Buffers);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		ERROR_LOG("alGenBuffers :" << error << "\n");
		return;
	}

	// Load test.wav
	AudioSource audio = {};
	int filesize = 0;
	get_filesize("data/audio/test.wav", &filesize);
	
	if (filesize > 0)
	{
		char* buffer = new char[filesize];
		if (read_buffer("data/audio/test.wav", buffer, filesize) != -1)
		{
			audio = load_wav(buffer, filesize);
			if(audio.data == nullptr)
				return;
		}
	}

	// Copy test.wav data into AL Buffer 0
	alBufferData(g_Buffers, audio.format, audio.data, audio.size_bytes, audio.freq);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		ERROR_LOG("alBufferData buffer 0 : " << error << "\n");
		alDeleteBuffers(1, &g_Buffers);
		return;
	}
	
	//unload test.wav
	delete[] audio.data;


	// Generate Sources
	alGenSources(1, &source);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		ERROR_LOG("alGenSources 1 : " << error << "\n");
		return;
	}

	// Attach buffer 0 to source 
	alSourcei(source, AL_BUFFER, g_Buffers);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		ERROR_LOG("alSourcei AL_BUFFER 0 : " << error << "\n");
	}
	
	/*
	// Exit
	Context = alcGetCurrentContext();
	Device = alcGetContextsDevice(Context);
	alcMakeContextCurrent(NULL);
	alcDestroyContext(Context);
	alcCloseDevice(Device);
	*/
}