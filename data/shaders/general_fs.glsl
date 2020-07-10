#version 400

in vec4 outNormal;
in vec4 outPosition;
in vec2 uvOut;
out vec4 fragmentColor;

uniform vec4 global_light;

uniform sampler2D diffuse_texture_0;
uniform sampler2D diffuse_texture_1;
uniform sampler2D diffuse_texture_2;
uniform sampler2D diffuse_texture_3;
uniform sampler2D diffuse_texture_4;
uniform sampler2D diffuse_texture_5;
uniform sampler2D diffuse_texture_6;
uniform sampler2D diffuse_texture_7;
uniform sampler2D diffuse_texture_8;
uniform sampler2D diffuse_texture_9;
uniform sampler2D diffuse_texture_10;
uniform sampler2D diffuse_texture_11;
uniform sampler2D diffuse_texture_12;
uniform sampler2D diffuse_texture_13;
uniform sampler2D diffuse_texture_14;
uniform sampler2D diffuse_texture_15;

uniform int active_texture;

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
	vec4 color;
	switch(active_texture)
	{
		case 0:
		{
			color = texture(diffuse_texture_0, uvOut);
		}break;
		case 1:
		{
			color = texture(diffuse_texture_1, uvOut);
		}break;
		case 2:
		{
			color = texture(diffuse_texture_2, uvOut);
		}break;
		case 3:
		{
			color = texture(diffuse_texture_3, uvOut);
		}break;
		case 4:
		{
			color = texture(diffuse_texture_4, uvOut);
		}break;
		case 5:
		{
			color = texture(diffuse_texture_5, uvOut);
		}break;
		case 6:
		{
			color = texture(diffuse_texture_6, uvOut);
		}break;
		case 7:
		{
			color = texture(diffuse_texture_7, uvOut);
		}break;
		case 8:
		{
			color = texture(diffuse_texture_8, uvOut);
		}break;
		case 9:
		{
			color = texture(diffuse_texture_9, uvOut);
		}break;
		case 10:
		{
			color = texture(diffuse_texture_10, uvOut);
		}break;
		case 11:
		{
			color = texture(diffuse_texture_11, uvOut);
		}break;
		case 12:
		{
			color = texture(diffuse_texture_12, uvOut);
		}break;
		case 13:
		{
			color = texture(diffuse_texture_13, uvOut);
		}break;
		case 14:
		{
			color = texture(diffuse_texture_14, uvOut);
		}break;
		case 15:
		{
			color = texture(diffuse_texture_15, uvOut);
		}break;
	}

	fragmentColor = ((0.1 + 0.9 * light) * color);
}
