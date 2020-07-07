#ifndef LOG_FILE_HEADER
#define LOG_FILE_HEADER

#include "config.h"

#if defined(SHOW_DEBUG_LOGS)

#include <iostream>
#define DEBUG_LOG(x) { \
	std::cout << x; \
}

#define ERROR_LOG(x) { \
	std::cerr << "ERROR: " << x; \
}

#define DEBUG_CALL(x) x

#else

#define DEBUG_LOG(x) 

#define DEBUG_CALL(x) 

#define ERROR_LOG(x) 

#endif



#endif