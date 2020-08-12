#ifndef READFILE_HEADER
#define READFILE_HEADER

#include <string>
#include <iostream>
#include <fstream>

#include "logfile.h"
#include "config.h"

#ifdef TEST_READFILE
void test_readfile();
#endif
void get_filesize(std::string filepath, int* filesize);

int read_buffer(std::string filepath, void* buffer, int buffer_size);

int read_buffer_offset(std::string filepath, int file_offset, void* buffer, int buffer_size);

//#define READFILE_IMPLEMENTATION
#ifdef READFILE_IMPLEMENTATION

#ifdef TEST_READFILE
void test_readfile()
{
	int buffer_size = 0;
	char* buffer;
	std::string filepath = "data/testfile";


	get_filesize(filepath, &buffer_size);
	buffer = new char[buffer_size];
	read_buffer(filepath, buffer, buffer_size);

	for (int i = 0; i < buffer_size; i++)
	{
		DEBUG_LOG(buffer[i]);
	}
	DEBUG_LOG("\n");

	delete[] buffer;
}
#endif

void get_filesize(std::string filepath, int* filesize)
{
	std::ifstream in;
	in.open(filepath, std::ios::in | std::ios::binary | std::ios::ate);
	if (in.is_open())
	{
		*filesize = in.tellg();
		in.close();
	}
	else
	{
		//Unable to open file
		ERROR_LOG("Unable to open file: " << filepath << "\n");
		*filesize = -1;
	}
}

int read_buffer(std::string filepath, void* buffer, int buffer_size)
{
	std::ifstream in;
	in.open(filepath, std::ios::in | std::ios::binary);
	if (in.is_open())
	{
		in.read((char*)buffer, buffer_size);
		in.close();
		return 0;
	}
	else
	{
		return -1;
	}
}

int read_buffer_offset(std::string filepath, int file_offset, void* buffer, int buffer_size)
{
	std::ifstream in;
	in.open(filepath, std::ios::in | std::ios::binary);
	if (in.is_open())
	{
		in.seekg(file_offset);
		in.read((char*)buffer, buffer_size);
		in.close();
		return 0;
	}
	else
	{
		return -1;
	}
}
#endif


#endif