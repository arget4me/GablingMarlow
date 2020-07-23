#include "wav_loader.h"

#include "Utils/logfile.h"

AudioSource load_wav(char* buffer, int buffersize)
{
	AudioSource audio = {};
	audio.data = nullptr;
	if (buffersize > 44)
	{
		//check riff chunk descriptor
		if (buffer[0] == 'R' &&
			buffer[1] == 'I' &&
			buffer[2] == 'F' &&
			buffer[3] == 'F')
		{
			int chunksize = *(int*)(buffer + 4);

			if (buffersize - chunksize == 8)
			{
				if (buffer[8 + 0] == 'W' &&
					buffer[8 + 1] == 'A' &&
					buffer[8 + 2] == 'V' &&
					buffer[8 + 3] == 'E')
				{
					if (buffer[12 + 0] == 'f' &&
						buffer[12 + 1] == 'm' &&
						buffer[12 + 2] == 't' &&
						buffer[12 + 3] == ' ')
					{
						int subchunk1size = *(int*)(buffer + 16);
						if (subchunk1size == 16)
						{
							short audio_format = *(short*)(buffer + 20);
							short num_channels = *(short*)(buffer + 22);
							int sample_rate = *(int*)(buffer + 24);
							int byte_rate = *(int*)(buffer + 28);
							short block_align = *(short*)(buffer + 32);
							short bits_per_sample = *(short*)(buffer + 34);

							audio.freq = sample_rate;
										
							if (num_channels == 1)
							{
								if (bits_per_sample == 8)
								{
									audio.format = AL_FORMAT_MONO8;


								}
								else if (bits_per_sample == 16)
								{

									audio.format = AL_FORMAT_MONO16;
								}
							}
							else if (num_channels == 2)
							{
								if (bits_per_sample == 8)
								{
									audio.format = AL_FORMAT_STEREO8;
									
										
								}
								else if (bits_per_sample == 16)
								{

									audio.format = AL_FORMAT_STEREO16;
								}
							}

							if (36 + 8 < buffersize)
							{
								if (buffer[36 + 0] == 'd' &&
									buffer[36 + 1] == 'a' &&
									buffer[36 + 2] == 't' &&
									buffer[36 + 3] == 'a')
								{
									int subchunk2size = *(int*)(buffer + 40);

									if (44 + subchunk2size == buffersize)
									{
										audio.size_bytes = subchunk2size;
										audio.data = new char[audio.size_bytes];
										for (int i = 0; i < audio.size_bytes; i++)
										{
											//Copy entire data
											audio.data[i] = buffer[44 + i];
										}
									}
								}
							}
						}
					}
				}
			}
		}

	}

	return audio;
}