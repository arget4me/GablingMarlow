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
	//fragmentColor = normalize(outNormal);
	fragmentColor = (0.1 + 0.9 * light) *texture(diffuse_texture_0, uvOut);//vec4(clamp((outNormal.xyz + 1.0) / 4.0, 0.0, 0.6) + clamp(( outPosition.xyz + 1.0 ) * 0.2, 0.0, 0.4), 1.0);
}
