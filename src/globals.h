#ifndef GLOBALS_HEADER
#define GLOBALS_HEADER




//#define GLOBALS_DEFINITIONS
#ifdef GLOBALS_DEFINITIONS
extern int global_height = 720;
extern int global_width = 1280;
bool keys[4] = {//W A S D
	false,
	false,
	false,
	false,
};

#else
extern int global_height;
extern int global_width;
bool keys[];
#endif // GLOBALS_DEFINITIONS



#endif // !GLOBALS_HEADER
