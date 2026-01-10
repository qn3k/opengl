#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords; // To musi być!
layout (location = 2) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProj;

void main() {
    FragPos = vec3(matModel * vec4(aPos, 1.0));
    // Przekazywanie normalnych (ważne dla oświetlenia)
    Normal = mat3(transpose(inverse(matModel))) * aNormal;  
    TexCoords = aTexCoords;
    
    gl_Position = matProj * matView * vec4(FragPos, 1.0);
}