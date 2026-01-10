#version 330 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;     
layout(location = 2) in vec3 inNormal; 

out vec3 FragNormal;
out vec3 FragPos;

uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProj;

void main()
{
    gl_Position = matProj * matView * matModel * vec4(inPosition, 1.0);

    FragNormal = mat3(transpose(inverse(matModel))) * inNormal;
    FragPos = vec3(matModel * vec4(inPosition, 1.0));
}