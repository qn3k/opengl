#version 330 core
layout( location = 0 ) in vec4 inPosition;

out vec4 fragPos;
uniform mat4 matProj;
uniform mat4 matView;
uniform mat4 matModel;

void main()
{
	gl_Position = matProj * matView * matModel * inPosition;
	fragPos = inPosition;
}
