#version 400

in vec4 outNormal;
in vec4 outPosition;
in vec4 baseColor;
out vec4 fragmentColor;

uniform vec4 global_light;// = vec4(0.0f, 0.0f, 10.0f, 1.0f);

void main()
{
	
	float light = dot(normalize(global_light - outPosition), outNormal);
	if(light <= 0.1)
	{
		light = 0;
	}
	else if(light <= 0.25)
	{
		light = 0.25;
	}
	else if(light <= 0.5)
	{
		light = 0.5;
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
	fragmentColor = (0.1 + 0.9 * light) * baseColor;//vec4(clamp((outNormal.xyz + 1.0) / 4.0, 0.0, 0.6) + clamp(( outPosition.xyz + 1.0 ) * 0.2, 0.0, 0.4), 1.0);
}
