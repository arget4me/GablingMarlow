#version 400

in vec4 outNormal;
in vec4 outPosition;
out vec4 fragmentColor;


void main()
{
	fragmentColor  = vec4((outNormal.xyz + 1.0) / 4.0 + ( outPosition.xyz + 1.0 ) * 0.2, 1.0);
}
