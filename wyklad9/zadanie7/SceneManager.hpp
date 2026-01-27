#ifndef SCENE_MANAGER_HPP
#define SCENE_MANAGER_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "collider.hpp"

// --- STRUKTURY DANYCH ---

extern std::vector<glm::mat4> treeMatrices;

struct Material {
    float ambient;
    float diffuse;
    float specular;
    float shininess;
};

struct PlayerIndices {
    int body = -1;
    int legR = -1;
    int legL = -1;
};
extern PlayerIndices pIdx;

//podloga z heightmap
struct TerrainVertex {
    glm::vec3 position;
    glm::vec2 texCoords;
    glm::vec3 normal;
};
//pobieranie wysokosci z ground jako obj
float GetHeight(float x, float z);

class CMesh {
public:
    GLuint idVAO = 0;
    GLuint idVBO_pos, idVBO_norm, idVBO_uv;
    GLuint idVBO_instance = 0; // Nowy bufor na macierze instancji
    GLuint idEBO = 0;
    std::vector<glm::vec3> vertices;

    int vertexCount = 0;
    bool usesIndices = false; //do heightmapa
    int instanceCount = 0; //liczba obiektow 
    bool isInstanced = false;  // Flaga czy używamy instancjonowania

    bool Load(const char* path);
    void Draw();
    void Release();
    static CMesh CreateFlowerMesh();
    void CreateFromHeightmap(const std::vector<TerrainVertex>& vertices, const std::vector<unsigned int>& indices);
    void PrepareInstancing(const std::vector<glm::mat4>& matrices, const std::vector<glm::vec3>& colors);
};

struct SceneObject {
    CMesh* mesh;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    glm::vec3 color;
    GLuint idTexture; 
    Material mat;     
    CCollider* collider = nullptr; //kolizje
};

// --- PROTOTYPY FUNKCJI ---
GLuint LoadTexture(const char* path, bool flip = true);
GLuint LoadCubemap(std::vector<std::string> faces);
void InitializeResources(std::vector<CMesh>& meshes, std::vector<GLuint>& textures);
void BuildScene(std::vector<SceneObject>& scene, std::vector<CMesh>& meshes, std::vector<GLuint>& textures);
void LoadHeightmap(const char* filename, std::vector<TerrainVertex>& vertices, std::vector<unsigned int>& indices);
void GenerateNormals(std::vector<TerrainVertex>& vertices, int width, int height);

#endif