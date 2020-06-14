#version 400

in vec4 outNormal;
out vec4 fragmentColor;


void main()
{
	fragmentColor  = vec4((outNormal.xyz + 1.0) / 2.0, 1.0);
}
