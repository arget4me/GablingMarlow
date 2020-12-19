#version 400

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
uniform mat4 mvp;
uniform mat4 m;
uniform float water_scroll;
out vec4 outNormal;
out vec4 outPosition;
out vec2 uvOut;
out float depth;

void main()
{
	vec4 pos = vec4(position, 1);
	if(pos.y < 0)
		pos.y = 0.2*sin(pos.x*pos.x + pos.z*pos.z +water_scroll); 
	gl_Position = mvp * pos;
	outNormal = normalize(m * vec4(normal, 0));
	outPosition = m * pos;
	uvOut = vec2(uv.x, -uv.y);
	depth = (mvp * vec4(position, 1)).z;
}
