#version 400

in vec4 outNormal;
in vec4 outPosition;
in vec2 uvOut;
in float depth;
out vec4 fragmentColor;

uniform vec4 global_light;

uniform vec4 color_dark;
uniform vec4 color_mid;
uniform vec4 color_light;

uniform vec2 displacement_offset;
uniform vec2 mask_offset;


uniform sampler2D diffuse_texture_0;
uniform sampler2D diffuse_texture_1;

void main()
{
	vec2 displacement = texture(diffuse_texture_1, uvOut + displacement_offset).xy;
	vec2 displacement_2 = texture(diffuse_texture_1, vec2(0.2, -0.4) + uvOut + displacement_offset).xy;
	vec4 color = texture(diffuse_texture_0, uvOut + (displacement /50.0) + mask_offset);
	vec4 color_2 = texture(diffuse_texture_0, vec2(0.2, -0.4) + uvOut + (displacement_2 /54.0) + mask_offset);

	if(color.x == 0)
	{
		color  = color_2.x * color_mid  + (1.0f - color_2.x) * color_dark;
	}
	else
	{
		color = color.x * color_light  + (1.0f - color.x) * color_dark;
	}

	float blend_value = depth/400.0f;
	if(blend_value < 0.0)blend_value = 0.0;
	if(blend_value > 1.0)blend_value = 1.0;
	blend_value = blend_value * blend_value * blend_value;
	fragmentColor = blend_value * color_dark + (1 - blend_value) * color;
}
