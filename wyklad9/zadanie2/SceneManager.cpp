#include "SceneManager.hpp"
#include "objloader.hpp"
#include "stb_image.h"
#include <stdio.h>

// Implementacja metod CMesh
bool CMesh::Load(const char* path) {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    if (!loadOBJ(path, vertices, uvs, normals)) return false;

    vertexCount = vertices.size();
    glGenVertexArrays(1, &idVAO);
    glBindVertexArray(idVAO);

    glGenBuffers(1, &idVBO_pos);
    glBindBuffer(GL_ARRAY_BUFFER, idVBO_pos);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &idVBO_norm);
    glBindBuffer(GL_ARRAY_BUFFER, idVBO_norm);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(2);
    
    if (!uvs.empty()) {
        glGenBuffers(1, &idVBO_uv);
        glBindBuffer(GL_ARRAY_BUFFER, idVBO_uv);
        glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(1);
    }
    glBindVertexArray(0);
    this->usesIndices = false; // Pliki OBJ loadera używają DrawArrays
    return true;
}

void CMesh::Draw() { 
    if (!idVAO) return;

    glBindVertexArray(idVAO); 
    
    if (usesIndices) {
        // Używane dla Heightmapy
        glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, 0);
    } else {
        // Używane dla plików OBJ i Kwiatów
        glDrawArrays(GL_TRIANGLES, 0, vertexCount); 
    }
    
    glBindVertexArray(0);
}

void CMesh::Release() { 
    glDeleteBuffers(1, &idVBO_pos); 
    glDeleteBuffers(1, &idVBO_norm); 
    glDeleteBuffers(1, &idVBO_uv); 
    if (usesIndices) glDeleteBuffers(1, &idEBO); // dodano do obslugi heightmap
    glDeleteVertexArrays(1, &idVAO); 
}

CMesh CMesh::CreateFlowerMesh() {
    CMesh mesh;
    GLfloat vertices[12*3] = {
        -1.0, -1.0, 0.0, 1.0, 1.0, 0.0, 1.0, -1.0, 0.0,
        -1.0, -1.0, 0.0, -1.0, 1.0, 0.0, 1.0, 1.0, 0.0,
        0.0, -1.0, 1.0, 0.0, 1.0, -1.0, 0.0, -1.0, -1.0,
        0.0, -1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, -1.0,
    };
    GLfloat uvs[12*2] = {
        0.00, 0.00, 0.98, 0.98, 0.98, 0.00,
        0.00, 0.00, 0.00, 0.98, 0.98, 0.98,
        0.98, 0.00, 0.00, 0.98, 0.00, 0.00,
        0.98, 0.00, 0.98, 0.98, 0.00, 0.98,
    };
    GLfloat normals[12*3];
    for(int i=0; i<6; ++i) { normals[i*3] = 0.0f; normals[i*3+1] = 0.0f; normals[i*3+2] = 1.0f; }
    for(int i=6; i<12; ++i) { normals[i*3] = 1.0f; normals[i*3+1] = 0.0f; normals[i*3+2] = 0.0f; }

    mesh.vertexCount = 12;
    glGenVertexArrays(1, &mesh.idVAO);
    glBindVertexArray(mesh.idVAO);
    glGenBuffers(1, &mesh.idVBO_pos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.idVBO_pos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    glGenBuffers(1, &mesh.idVBO_norm);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.idVBO_norm);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(2);
    glGenBuffers(1, &mesh.idVBO_uv);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.idVBO_uv);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    return mesh;
}

GLuint LoadTexture(const char* path) {
    GLuint textureID;
    int tex_width, tex_height, tex_n;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *tex_data = stbi_load(path, &tex_width, &tex_height, &tex_n, 0);
    if (tex_data == NULL) return 0;
    GLenum format = (tex_n == 4) ? GL_RGBA : GL_RGB;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //dodano do podlogi
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //to tez do podlogi
    glTexImage2D(GL_TEXTURE_2D, 0, format, tex_width, tex_height, 0, format, GL_UNSIGNED_BYTE, tex_data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(tex_data);
    return textureID;
}

void InitializeResources(std::vector<CMesh>& meshes, std::vector<GLuint>& textures) {
    CMesh mGround, mCube, mSphere, mMonkey,mKoliber ,mFlower, mTerrain;
    mGround.Load("obj/cube.obj"); 
    mCube.Load("obj/cube.obj");
    mSphere.Load("obj/sphere.obj");
    mMonkey.Load("obj/monkey.obj");
    mKoliber.Load("obj/koliber.obj");
    mFlower = CMesh::CreateFlowerMesh();

    //do heightmapa
    std::vector<TerrainVertex> terrainVerts;
    std::vector<unsigned int> terrainIndices;
    // 1. Wczytujemy dane z obrazka 
    LoadHeightmap("textures/heightmap.png", terrainVerts, terrainIndices);
    // 2. Obliczamy wektory normalne, aby pagórki reagowały na światło
    GenerateNormals(terrainVerts, 256, 256);
    // 3. Tworzymy VAO/VBO dla terenu
    mTerrain.CreateFromHeightmap(terrainVerts, terrainIndices);

    meshes.push_back(mGround); // 0
    meshes.push_back(mCube);   // 1
    meshes.push_back(mSphere); // 2
    meshes.push_back(mMonkey); // 3
    meshes.push_back(mFlower); // 4
    meshes.push_back(mKoliber); //5
    meshes.push_back(mTerrain); //6 

    textures.push_back(0); // 0
    textures.push_back(LoadTexture("textures/grass.png")); 
    textures.push_back(LoadTexture("textures/metal.png")); 
    textures.push_back(LoadTexture("textures/brick.png")); 
    textures.push_back(LoadTexture("textures/flower32bit.png"));
    textures.push_back(LoadTexture("textures/sand.png"));
}

void BuildScene(std::vector<SceneObject>& scene, std::vector<CMesh>& meshes, std::vector<GLuint>& textures) {

    // --- OBIEKT 1: PODŁOGA Z HEIGHTMAPY ---
    SceneObject terrain;
    terrain.mesh = &meshes[6]; // Używamy mTerrain
    terrain.position = glm::vec3(0.0f, -22.0f, 0.0f); // Obniżamy nieco, by obiekty nie były pod ziemią
    terrain.rotation = glm::vec3(0.0f);
    terrain.scale = glm::vec3(1.0f); // Skala 1.0, bo LoadHeightmap już używa xzScale
    terrain.color = glm::vec3(1.0f, 0.0f, 1.0f); 
    terrain.idTexture = textures[5]; // Używamy piasku
    terrain.mat = {0.2f, 0.8f, 0.1f, 2.0f}; // Materiał matowy
    scene.push_back(terrain);

    // --- OBIEKT 2: Sześcian ---
    SceneObject box;
    box.mesh = &meshes[1];
    box.position = glm::vec3(-2.0f, 0.0f, -2.0f);
    box.rotation = glm::vec3(0.0f);
    box.scale = glm::vec3(1.0f);
    box.color = glm::vec3(1.0f); 
    box.idTexture = textures[3]; // Cegły
    box.mat = {0.2f, 0.7f, 0.2f, 10.0f}; // Lekki połysk
    scene.push_back(box);

    // --- OBIEKT 3: Sfera (Najlepsza do testu Blinn-Phong!) ---
    // --- SFERA 1: BŁYSZCZĄCA (Shiny) ---
    SceneObject shinyBall;
    shinyBall.mesh = &meshes[2]; // Sfera
    shinyBall.position = glm::vec3(-2.5f, 1.0f, 0.0f);
    shinyBall.scale = glm::vec3(1.0f);
    shinyBall.color = glm::vec3(0.0f, 0.4f, 1.0f); // Niebieska
    shinyBall.idTexture = 0; // Bez tekstury, żeby lepiej widzieć blask
    // Materiał: wysoki specular (1.0) i mocne skupienie (128.0)
    shinyBall.mat = { 0.1f, 0.5f, 1.0f, 128.0f }; 
    scene.push_back(shinyBall);

    // --- SFERA 2: MATOWA (Matte) ---
    // Reaguje tylko na diffuse, nie ma "plamy" światła odbitego
    SceneObject matteBall;
    matteBall.mesh = &meshes[2]; // Ta sama siatka sfery
    matteBall.position = glm::vec3(2.5f, 1.0f, 0.0f);
    matteBall.scale = glm::vec3(1.0f);
    matteBall.color = glm::vec3(1.0f, 0.3f, 0.0f); // Pomarańczowa
    matteBall.idTexture = 0;
    // Materiał: specular ustawiony na 0.0 całkowicie wyłącza lśnienie
    matteBall.mat = { 0.2f, 0.8f, 0.0f, 1.0f }; 
    scene.push_back(matteBall);

    // --- OBIEKT 4: Małpka ---
    SceneObject suzanne;
    suzanne.mesh = &meshes[3]; 
    suzanne.position = glm::vec3(0.0f, 0.5f, 1.0f);
    suzanne.rotation = glm::vec3(0.0f, 3.14f, 0.0f); 
    suzanne.scale = glm::vec3(1.0f);
    suzanne.color = glm::vec3(1.0f, 0.8f, 0.1f); 
    suzanne.idTexture = 0;
    suzanne.mat = {0.2f, 0.8f, 0.5f, 32.0f}; // Złoty połysk
    scene.push_back(suzanne);

    // --- OBIEKT 5: KWIATY ---
    SceneObject flower1;
    flower1.mesh = &meshes[4]; 
    flower1.position = glm::vec3(4.0f, -0.5f, -3.0f); 
    flower1.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    flower1.scale= glm::vec3(0.5f, 0.5f, 0.5f);
    flower1.color = glm::vec3(1.0f); 
    flower1.idTexture = textures[4]; // Tekstura kwiatu z alfą
    flower1.mat = {0.4f, 0.6f, 0.0f, 1.0f}; // Brak połysku na płatkach
    scene.push_back(flower1);
    
    SceneObject flower2 = flower1;
    flower2.position = glm::vec3(-3.0f, -0.5f, 3.0f);
    flower2.rotation = glm::vec3(0.0f, 1.57f, 0.0f); 
    scene.push_back(flower2);

    // --- OBIEKT 6: KOLIBER ---
    SceneObject koliber;
    koliber.mesh = &meshes[5]; 
    koliber.position = glm::vec3(0.0f, 5.0f, 0.0f); // Wyżej nad ziemią
    koliber.rotation = glm::vec3(0.0f, 1.57f, 0.0f);
    koliber.scale = glm::vec3(2.0f); 
    koliber.color = glm::vec3(0.1f, 0.5f, 0.4f); 
    koliber.idTexture = 0;
    koliber.mat = {0.2f, 0.8f, 1.0f, 128.0f}; 
    scene.push_back(koliber);

}

void LoadHeightmap(const char* filename, std::vector<TerrainVertex>& vertices, std::vector<unsigned int>& indices) {
    int width, height, channels;
    // Wczytujemy obrazek jako czarno-biały (1 kanał - STBI_grey)
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 1);
    
    if (!data) {
        printf("Błąd wczytywania heightmapy: %s\n", filename);
        return;
    }

    float yScale = 0.2f; // Jak wysokie mają być góry
    float xzScale = 1.0f; // Odstęp między punktami

    // 1. Generowanie wierzchołków
    for (int z = 0; z < height; z++) {
        for (int x = 0; x < width; x++) {
            float y = data[z * width + x] * yScale;
            
            TerrainVertex v;
            // Centrujemy teren, odejmując połowę szerokości
            v.position = glm::vec3((x - width/2.0f) * xzScale, y, (z - height/2.0f) * xzScale);
            v.texCoords = glm::vec2((float)x / width, (float)z / height);
            v.normal = glm::vec3(0.0f, 1.0f, 0.0f); // Uproszczony normal (do poprawy później dla światła)
            
            vertices.push_back(v);
        }
    }

    // 2. Generowanie indeksów (budowanie dwóch trójkątów dla każdego kwadratu siatki)
    for (int z = 0; z < height - 1; z++) {
        for (int x = 0; x < width - 1; x++) {
            unsigned int topLeft = z * width + x;
            unsigned int topRight = topLeft + 1;
            unsigned int bottomLeft = (z + 1) * width + x;
            unsigned int bottomRight = bottomLeft + 1;

            // Pierwszy trójkąt
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            // Drugi trójkąt
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    stbi_image_free(data);
}

void CMesh::CreateFromHeightmap(const std::vector<TerrainVertex>& vertices, const std::vector<unsigned int>& indices) {
    this->usesIndices = true; // Teren używa indeksów (EBO)
    
    glGenVertexArrays(1, &idVAO);
    glBindVertexArray(idVAO);

    GLuint vbo, ebo;
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &idEBO); // Zapisujemy do składowej klasy

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(TerrainVertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Atrybuty (pozycja, uv, normalne) - tak jak pisałeś wcześniej
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void*)offsetof(TerrainVertex, texCoords));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void*)offsetof(TerrainVertex, normal));
    glEnableVertexAttribArray(2);

    this->vertexCount = indices.size(); 
    glBindVertexArray(0);
}

void GenerateNormals(std::vector<TerrainVertex>& vertices, int width, int height) {
    for (int z = 0; z < height; z++) {
        for (int x = 0; x < width; x++) {
            // Pobieramy wysokości sąsiadów (z obsługą krawędzi)
            float hL = vertices[z * width + std::max(0, x - 1)].position.y; 
            float hR = vertices[z * width + std::min(width - 1, x + 1)].position.y;
            float hD = vertices[std::max(0, z - 1) * width + x].position.y;
            float hU = vertices[std::min(height - 1, z + 1) * width + x].position.y;

            // Formuła na wektor normalny na podstawie różnicy wysokości
            glm::vec3 normal;
            normal.x = hL - hR;
            normal.y = 2.0f; // Im większa wartość, tym teren wydaje się "płaski"
            normal.z = hD - hU;
            vertices[z * width + x].normal = glm::normalize(normal);
        }
    }
}