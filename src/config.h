#ifndef CONFIG_HEADER
#define CONFIG_HEADER

#define SHOW_DEBUG_LOGS
#define FPS_TIMED

#define global_scope extern
#define local_scope static


//#define TEST_READFILE

//#define TEST_LOADOBJ
#ifdef TEST_LOADOBJ
#define TEST_MODEL "data/models/dice.obj"
//#define TEST_MODEL "data/models/dice_duplicate.obj"
//#define TEST_MODEL "data/models/dice_no_uv.obj"
#endif


#endif