#ifndef CONFIG_HEADER
#define CONFIG_HEADER

#if defined(_DEBUG)
#define SHOW_DEBUG_LOGS
#endif

#define FPS_TIMED

#define global_scope extern
#define local_scope static

//#define TEST_READFILE
//#define TEST_LOADOBJ
#define TEST_LOADDAE

#define RECORDNING_MODE 0

#if !RECORDNING_MODE
	#define START_IN_FULLSCREEN 0
	#define VSYNC_ON 0
	#define CONVERT_OBJ 0
#else
	#define START_IN_FULLSCREEN 1
	#define VSYNC_ON 1

	#define CONVERT_OBJ 0

	#define TEST_LOADDAE
	#undef FPS_TIMED
	#undef TEST_READFILE
	#undef TEST_LOADOBJ
#endif



#ifdef TEST_LOADOBJ
#define TEST_MODEL "data/models/dice.obj"
//#define TEST_MODEL "data/models/dice_duplicate.obj"
//#define TEST_MODEL "data/models/dice_no_uv.obj"
#endif


#ifdef TEST_LOADDAE
#define TEST_DAE_FILE "data/models/prototype_tree_animated.dae"
//#define TEST_DAE_FILE "data/models/test_04.dae"
#endif





#endif