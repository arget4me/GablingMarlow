#version 400

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
uniform mat4 mvp;
uniform mat4 mv;
out vec4 outNormal;
out vec4 outPosition;
out vec2 uvOut;

void main()
{
	gl_Position = mvp * vec4(position, 1);
	outNormal = normalize(mv * vec4(normal, 0));
	outPosition = mv * vec4(position, 1);
	uvOut = vec2(uv.x, -uv.y);
}
