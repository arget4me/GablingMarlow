#ifndef CONFIG_HEADER
#define CONFIG_HEADER

#if defined(_DEBUG)
#define SHOW_DEBUG_LOGS
#endif

#define FPS_TIMED

#define global_scope extern
#define local_scope static

#define START_IN_FULLSCREEN 0

//#define TEST_READFILE

//#define TEST_LOADOBJ
#ifdef TEST_LOADOBJ
#define TEST_MODEL "data/models/dice.obj"
//#define TEST_MODEL "data/models/dice_duplicate.obj"
//#define TEST_MODEL "data/models/dice_no_uv.obj"
#endif

#define TEST_LOADDAE
#ifdef TEST_LOADDAE
#define TEST_DAE_FILE "data/models/prototype_tree_animated.dae"
//#define TEST_DAE_FILE "data/models/test_04.dae"

#endif

#endif