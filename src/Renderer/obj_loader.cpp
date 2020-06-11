#include "obj_loader.h"
#include <Utils/readfile.h>

void test_loadobj()
{
	int filesize;
	char* buffer;
	std::string test_model_path = "data/models/dice.obj";
	get_filesize(test_model_path, &filesize);
	if (filesize != -1)
	{
		buffer = new char[filesize + 1];
		buffer[filesize] = '\0';
		if (read_buffer(test_model_path, buffer, filesize) != -1)
		{
			/*
			for (int i = 0; i < filesize; i++)
			{
				DEBUG_LOG(buffer[i]);
			}
			DEBUG_LOG("\n");
			*/
		}
		loadobj(buffer, filesize);
		delete[] buffer;
	}

}


void separate_tokens(char* buffer, int buffersize)
{
	for (int i = 0; i < buffersize; i++)
	{
		if (buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == '\r')
		{
			buffer[i] = '\0';
		}
	}
}

int get_token_size(char* buffer, int buffersize, int current_location)
{
	int token_size = 0;
	int i = current_location;
	while (i < buffersize)
	{
		if (buffer[i] == '\0')
			break;
		i++;
		token_size++;
	}

	return token_size;
}

void loadobj(char* buffer, int buffersize)
{
	separate_tokens(buffer, buffersize);
	int current_location = 0;
	while (current_location < buffersize)
	{
		int token_size = get_token_size(buffer, buffersize, current_location);
		char* token = buffer + current_location;
		DEBUG_LOG(token << "\n");
		current_location += token_size;
		while((current_location < buffersize) && (buffer[current_location] == '\0'))
		{
			current_location++;
		}
	}





}