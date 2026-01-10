#version 330 core
out vec4 FragColor;

struct Material {
    float ambient;
    float diffuse;
    float specular;
    float shininess;
};

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 uColor;
uniform sampler2D uTextureSampler;
uniform bool bUseTexture;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform bool useLighting;
uniform bool useBlinnPhong;
uniform Material material;

void main() {
    vec3 baseColor = bUseTexture ? texture(uTextureSampler, TexCoord).rgb : uColor;
    
    if(!useLighting) {
        FragColor = vec4(baseColor, 1.0);
        return;
    }

    // 1. Ambient
    vec3 ambient = material.ambient * baseColor;

    // 2. Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = material.diffuse * diff * baseColor;

    // 3. Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    float spec = 0.0;
    
    if(useBlinnPhong) {
        // Model Blinna-Phonga (Halfway vector)
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    } else {
        // Model Phonga (Reflection vector)
        vec3 reflectDir = reflect(-lightDir, norm);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }
    
    vec3 specular = material.specular * spec * vec3(1.0); // Białe odbicie

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}