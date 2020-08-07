#version 400

in vec4 outNormal;
in vec4 outPosition;
in vec2 uvOut;
out vec4 fragmentColor;

uniform vec4 global_light;

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
	fragmentColor = ((0.1 + 0.9 * light) * color);
}
