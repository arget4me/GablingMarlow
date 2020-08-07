#version 400

in vec4 outPosition;
out vec4 fragmentColor;
uniform vec4 sky_color;

void main()
{
	float blend_value = outPosition.y;
	if(blend_value < 0.0)blend_value = 0.0;
	if(blend_value > 1.0)blend_value = 1.0;
	vec4 color_blend = vec4(sky_color.xyz * 0.2, 1.0);
	fragmentColor = blend_value * color_blend + (1 - blend_value) * sky_color;
}
