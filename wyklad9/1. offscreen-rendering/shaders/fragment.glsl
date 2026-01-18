#version 150

in vec4 inoutPos;
in vec2 inoutUV;

out vec4 outColor;

uniform sampler2D tex;

void main()
{

	outColor = texture( tex, inoutUV );

}
