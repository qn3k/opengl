#ifndef SCENE_MANAGER_HPP
#define SCENE_MANAGER_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

// --- STRUKTURY DANYCH ---

struct Material {
    float ambient;
    float diffuse;
    float specular;
    float shininess;
};

class CMesh {
public:
    GLuint idVAO = 0;
    GLuint idVBO_pos, idVBO_norm, idVBO_uv;
    int vertexCount = 0;

    bool Load(const char* path);
    void Draw();
    void Release();
    static CMesh CreateFlowerMesh();
};

struct SceneObject {
    CMesh* mesh;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    glm::vec3 color;
    GLuint idTexture; 
    Material mat;     
};

// --- PROTOTYPY FUNKCJI ---
GLuint LoadTexture(const char* path);
void InitializeResources(std::vector<CMesh>& meshes, std::vector<GLuint>& textures);
void BuildScene(std::vector<SceneObject>& scene, std::vector<CMesh>& meshes, std::vector<GLuint>& textures);

#endif