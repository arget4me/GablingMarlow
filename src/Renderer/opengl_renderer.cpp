#include "opengl_renderer.h"


void setup_gl_renderer()
{
	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
}

void draw()
{

}