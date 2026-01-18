#version 330

layout( location = 0 ) in vec4 inPosition;
layout( location = 1 ) in vec2 inUV;

uniform mat4 matPVM;

out vec4 inoutPos;
out vec2 inoutUV;


void main()
{
	gl_Position = matPVM * inPosition;

	inoutPos = inPosition;
	inoutUV = inUV;
}
