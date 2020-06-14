#version 400

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec2 uv;
uniform mat4 mvp;
uniform mat4 mv;
out vec4 outNormal;
out vec4 outPosition;

void main()
{
	gl_Position = mvp * position;
	outNormal = mv * normal;
	outPosition = mv * position;
}
