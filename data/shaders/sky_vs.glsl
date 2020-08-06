#version 400

layout(location = 0) in vec3 position;
uniform mat4 mvp;
uniform mat4 m;
out vec4 outPosition;

void main()
{
	vec4 pos = vec4(position, 1);
	outPosition = pos;
	gl_Position = mvp * pos;
}
