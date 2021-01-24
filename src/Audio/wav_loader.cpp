#include "wav_loader.h"
#include <Utils/logfile.h>

Wav_file load_wav(char* buffer, int buffersize)
{
	ERROR_LOG("Wav_loader nolonger works properly. Fix!!");
	
	/*@HACK: just changed to not give compile errors. Not tested an probably doesn't work*///return;
	Wav_file wav = {};

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
							wav.num_channels = *(short*)(buffer + 22);
							int sample_rate = *(int*)(buffer + 24);
							int byte_rate = *(int*)(buffer + 28);
							short block_align = *(short*)(buffer + 32);
							wav.bits_per_sample = *(short*)(buffer + 34);

							wav.freq = sample_rate;
										

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
										wav.size_bytes = subchunk2size;
										wav.data = new char[wav.size_bytes];
										for (int i = 0; i < wav.size_bytes; i++)
										{
											//Copy entire data
											wav.data[i] = buffer[44 + i];
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

	return wav;
}