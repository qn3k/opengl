#version 330

layout( location = 0 ) in vec4 inPosition;
layout( location = 1 ) in vec2 inUV;

uniform mat4 matProj;
uniform mat4 matView;
uniform mat4 matModel = mat4(1.0);

out vec4 inoutPos;
out vec2 inoutUV;


void main()
{
	gl_Position = matProj * matView * matModel * inPosition;

	inoutPos = inPosition;
	inoutUV = inUV;
}
