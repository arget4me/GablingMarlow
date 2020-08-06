#version 400

in vec4 outPosition;
out vec4 fragmentColor;
uniform vec4 color_dark;

void main()
{
	float blend_value = abs(outPosition.y);
	if(blend_value >1)blend_value = 1;
	if(blend_value < 0)blend_value = 0;
	vec4 color_blend = vec4(0.0, 0.0, 0.2, 1.0);
	fragmentColor = blend_value * color_blend + (1 - blend_value) * color_dark;
}
