#version 330 core
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inTexCoords;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in mat4 instanceMatrix; // kwiaty
layout (location = 7) in vec3 inInstanceColor;
layout (location = 11) in vec3 aTangent; //normal mapping


out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec3 InstanceColor; 
out vec4 FragPosLightSpace;
out mat3 TBN; // Macierz do przesyłu do fragment shadera 

uniform mat4 matProj;
uniform mat4 matView;
uniform mat4 matModel;
uniform mat4 lightProj;
uniform mat4 lightView;
uniform bool bIsInstanced;

void main()
{
    mat4 finalModel = bIsInstanced ? instanceMatrix : matModel;
    InstanceColor = bIsInstanced ? inInstanceColor : vec3(1.0);

    FragPos = vec3(finalModel * vec4(inPosition, 1.0));
    
    mat3 normalMatrix = mat3(transpose(inverse(finalModel)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * inNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    
    TBN = mat3(T, B, N);
    Normal = N; 
    
    TexCoords = inTexCoords;
    FragPosLightSpace = lightProj * lightView * vec4(FragPos, 1.0);
    gl_Position = matProj * matView * vec4(FragPos, 1.0);
}