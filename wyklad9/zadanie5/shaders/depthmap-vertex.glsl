#version 330 core
layout (location = 0) in vec3 inPosition;

uniform mat4 matProj;
uniform mat4 matView;
uniform mat4 matModel;

void main()
{
    gl_Position = matProj * matView * matModel * vec4(inPosition, 1.0);
}
