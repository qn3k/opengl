#version 150

in vec4 inoutPos;
in vec2 inoutUV;

out vec4 outColor;

uniform sampler2D tex0;


void main()
{
	outColor = texture( tex0, inoutUV );
}
