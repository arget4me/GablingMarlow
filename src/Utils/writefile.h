#ifndef WRITEFILE_HEADER
#define WRITEFILE_HEADER

#include <string>
#include <iostream>
#include <fstream>

#include "logfile.h"
#include "config.h"

int write_buffer_overwrite(std::string filepath, void* buffer, int buffer_size);

int write_buffer_append(std::string filepath, void* buffer, int buffer_size);


//#define WRITEFILE_IMPLEMENTATION
#ifdef WRITEFILE_IMPLEMENTATION

int write_buffer_overwrite(std::string filepath, void* buffer, int buffer_size)
{
	std::ofstream out;
	out.open(filepath, std::ios::out | std::ios::binary);
	if (out.is_open())
	{
		out.write((char*)buffer, buffer_size);
		out.close();
		return 0;
	}
	else
	{
		return -1;
	}
}

int write_buffer_append(std::string filepath, void* buffer, int buffer_size)
{
	std::ofstream out;
	out.open(filepath, std::ios::out | std::ios::binary | std::ios::app);
	if (out.is_open())
	{
		out.write((char*)buffer, buffer_size);
		out.close();
		return 0;
	}
	else
	{
		return -1;
	}
}

#endif


#endif