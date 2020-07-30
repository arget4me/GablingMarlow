#version 400

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in uvec4 bone_ids;
layout(location = 4) in vec4 bone_weights;

#define MAX_NUM_TRANSFORMS 100

uniform mat4 mvp;
uniform mat4 m;
uniform mat4 bone_transforms[MAX_NUM_TRANSFORMS];

out vec4 outNormal;
out vec4 outPosition;
out vec2 uvOut;

void main()
{
	vec4 pos = vec4(0.0);
	vec4 n = vec4(0.0);
	for(int i = 0; i < 4; i++)
	{
		pos += bone_weights[i] * bone_transforms[bone_ids[i]] * vec4(position, 1.0);
		n += bone_weights[i] * bone_transforms[bone_ids[i]] * vec4(normal, 0.0);
	}
	
	gl_Position = mvp * pos;
	outNormal = normalize(m * n);
	outPosition = m * pos;
	uvOut = vec2(uv.x, -uv.y);
}