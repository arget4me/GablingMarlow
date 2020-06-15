#version 400

in vec4 outNormal;
in vec4 outPosition;
out vec4 fragmentColor;


void main()
{
	fragmentColor  = vec4(clamp((outNormal.xyz + 1.0) / 4.0, 0.0, 0.6) + clamp(( outPosition.xyz + 1.0 ) * 0.2, 0.0, 0.4), 1.0);
}
