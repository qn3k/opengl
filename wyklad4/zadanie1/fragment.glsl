#version 330 core

in vec3 FragNormal;
in vec3 FragPos;

out vec4 outColor;

uniform vec3 uColor; 

void main()
{
    vec3 norm = normalize(FragNormal);

    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float diff = max(dot(norm, lightDir), 0.0);

    vec3 ambient = 0.3 * uColor;
    vec3 diffuse = diff * uColor;
    
    vec3 result = ambient + diffuse;

    outColor = vec4(result, 1.0);
}