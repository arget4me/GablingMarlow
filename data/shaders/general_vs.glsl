#version 400

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
uniform mat4 mvp;
uniform mat4 m;
out vec4 outNormal;
out vec4 outPosition;
out vec2 uvOut;
out float depth;

void main()
{
	gl_Position = mvp * vec4(position, 1);
	outNormal = normalize(m * vec4(normal, 0));
	outPosition = m * vec4(position, 1);
	uvOut = vec2(uv.x, -uv.y);
	depth = (mvp * vec4(position, 1)).z;
}
