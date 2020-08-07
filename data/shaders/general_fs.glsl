#version 400

in vec4 outNormal;
in vec4 outPosition;
in vec2 uvOut;
in float depth;
out vec4 fragmentColor;

uniform vec4 global_light;
uniform vec4 sky_color;

uniform sampler2D diffuse_texture_0;

void main()
{
	
	float light = dot(normalize(global_light - outPosition), outNormal);
	if(light <= 0.30)
	{
		light = 0.30;
	}
	else if(light <= 0.6)
	{
		light = 0.6;
	}
	else if(light <= 0.75)
	{
		light = 0.75;
	}
	else
	{
		light = 1.0;
	}
	vec4 color = texture(diffuse_texture_0, uvOut);

	if(color.a <= 0.01)
		discard;
	color = ((0.1 + 0.9 * light) * color);

	vec4 color_blend = vec4(sky_color.xyz * 0.2, 1.0);
	{
		float blend_value = outPosition.y / 300.0;
		if(blend_value < 0.0)blend_value = 0.0;
		if(blend_value > 1.0)blend_value = 1.0;
		color_blend = blend_value * color_blend + (1 - blend_value) * sky_color;
	}

	{
		float blend_value = depth/400.0f;
		if(blend_value < 0.0)blend_value = 0.0;
		if(blend_value > 1.0)blend_value = 1.0;
		blend_value = blend_value * blend_value * blend_value;


		fragmentColor = blend_value * color_blend + (1 - blend_value) * color;
	}
}
