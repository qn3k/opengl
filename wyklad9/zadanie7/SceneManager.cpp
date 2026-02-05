#include "objloader.hpp"
#include "SceneManager.hpp"
#include "stb_image.h"
#include <stdio.h>
#include <glm/gtc/matrix_transform.hpp>

std::vector<glm::mat4> flowerMatrices;
std::vector<glm::vec3> flowerColors;
const int FLOWER_COUNT = 500;
std::vector<glm::mat4> treeMatrices;
std::vector<glm::vec3> treeColors;
const int TREE_COUNT = 250;
//wysokosci do kwiatow
std::vector<float> globalHeights;
int hmWidth = 256;
int hmHeight = 256;
float hmYScale = 0.2f;
float hmXZScale = 1.0f;
PlayerIndices pIdx;

//obliczanie wysokosci do kwiatow z renderingu instancyjnego

// Funkcja pomocnicza do sprawdzania czy punkt jest wewnątrz trójkąta (rzut na XZ)
bool isPointInTriangle(glm::vec2 p, glm::vec2 a, glm::vec2 b, glm::vec2 c) {
    float det = (b.y - c.y) * (a.x - c.x) + (c.x - b.x) * (a.y - c.y);
    float l1 = ((b.y - c.y) * (p.x - c.x) + (c.x - b.x) * (p.y - c.y)) / det;
    float l2 = ((c.y - a.y) * (p.x - c.x) + (a.x - c.x) * (p.y - c.y)) / det;
    float l3 = 1.0f - l1 - l2;
    return l1 >= 0 && l2 >= 0 && l3 >= 0;
}

// Globalny wskaźnik na mesha podłogi (ustawimy go w InitializeResources)
CMesh* groundMeshPtr = nullptr;

float GetHeight(float x, float z) {
    if (!groundMeshPtr || groundMeshPtr->vertices.empty()) return 0.0f;

    // Definiujemy skale 
    float scaleX = 4.0f;
    float scaleY = 1.0f;
    float scaleZ = 4.0f;
    
    glm::vec2 p(x, z);

    for (size_t i = 0; i < groundMeshPtr->vertices.size(); i += 3) {
        glm::vec3 v1 = groundMeshPtr->vertices[i];
        v1.x *= scaleX; v1.y *= scaleY; v1.z *= scaleZ;

        glm::vec3 v2 = groundMeshPtr->vertices[i+1];
        v2.x *= scaleX; v2.y *= scaleY; v2.z *= scaleZ;

        glm::vec3 v3 = groundMeshPtr->vertices[i+2];
        v3.x *= scaleX; v3.y *= scaleY; v3.z *= scaleZ;

        // Sprawdzamy, czy gracz jest nad tym trójkątem (rzut na płaszczyznę XZ)
        if (isPointInTriangle(p, glm::vec2(v1.x, v1.z), glm::vec2(v2.x, v2.z), glm::vec2(v3.x, v3.z))) {
            
            // Obliczamy wyznacznik (determinant) dla współrzędnych barycentrycznych
            float det = (v2.z - v3.z) * (v1.x - v3.x) + (v3.x - v2.x) * (v1.z - v3.z);
            
            // Zabezpieczenie przed dzieleniem przez zero (trójkąt pionowy lub zdegenerowany)
            if (std::abs(det) < 0.00001f) continue;

            float l1 = ((v2.z - v3.z) * (x - v3.x) + (v3.x - v2.x) * (z - v3.z)) / det;
            float l2 = ((v3.z - v1.z) * (x - v3.x) + (v1.x - v3.x) * (z - v3.z)) / det;
            float l3 = 1.0f - l1 - l2;

            // Zwracamy wypadkową wysokość Y
            return l1 * v1.y + l2 * v2.y + l3 * v3.y;
        }
    }
    return -999999.0f; 
}

// Implementacja metod CMesh
bool CMesh::Load(const char* path) {
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> tangents; 

    // 1. Wczytujemy dane
    if (!loadOBJ(path, this->vertices, uvs, normals)) return false;

    // 2. obliczamy tangenty
    calculateTangents(this->vertices, uvs, normals, tangents);

    vertexCount = this->vertices.size();
    glGenVertexArrays(1, &idVAO);
    glBindVertexArray(idVAO);

    // VBO Pozycje (indeks 0)
    glGenBuffers(1, &idVBO_pos);
    glBindBuffer(GL_ARRAY_BUFFER, idVBO_pos);
    glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(glm::vec3), &this->vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    // VBO Normalne (indeks 2)
    glGenBuffers(1, &idVBO_norm);
    glBindBuffer(GL_ARRAY_BUFFER, idVBO_norm);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(2);
    
    // VBO UV (indeks 1)
    if (!uvs.empty()) {
        glGenBuffers(1, &idVBO_uv);
        glBindBuffer(GL_ARRAY_BUFFER, idVBO_uv);
        glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(1);
    }

    // VBO dla Tangentów (indeks 11)
    GLuint idVBO_tang;
    glGenBuffers(1, &idVBO_tang);
    glBindBuffer(GL_ARRAY_BUFFER, idVBO_tang);
    glBufferData(GL_ARRAY_BUFFER, tangents.size() * sizeof(glm::vec3), &tangents[0], GL_STATIC_DRAW);
    glVertexAttribPointer(11, 3, GL_FLOAT, GL_FALSE, 0, NULL); 
    glEnableVertexAttribArray(11);

    glBindVertexArray(0);
    this->usesIndices = false;
    return true;
}

void CMesh::Draw() { 
    if (!idVAO) return;

    glBindVertexArray(idVAO); 
    
    //rendering instancyjny
    if (isInstanced) {
        if (usesIndices) 
        // Używane dla Heightmapy
            glDrawElementsInstanced(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, 0, instanceCount);
        else 
        // Używane dla plików OBJ i Kwiatów
            glDrawArraysInstanced(GL_TRIANGLES, 0, vertexCount, instanceCount);
    } else {
        if (usesIndices) 
            glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, 0);
        else 
            glDrawArrays(GL_TRIANGLES, 0, vertexCount); 
    }
    
    glBindVertexArray(0);
}

void CMesh::Release() { 
    glDeleteBuffers(1, &idVBO_pos); 
    glDeleteBuffers(1, &idVBO_norm); 
    glDeleteBuffers(1, &idVBO_uv); 
    // glDeleteBuffers(1, &idVBO_tang); 
    if (usesIndices) glDeleteBuffers(1, &idEBO); //heightmap
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

GLuint LoadTexture(const char* path, bool flip) {
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(flip); 
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cout << "Nie wczytano tekstury: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

GLuint LoadCubemap(std::vector<std::string> faces) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void InitializeResources(std::vector<CMesh>& meshes, std::vector<GLuint>& textures) {
    CMesh mGround, mCube, mSphere, mMonkey,mKoliber ,mFlower, mTerrain, playerObj,
    playerBody, playerLegR, playerLegL,mTree, mWall_1,mWall_2;
    mGround.Load("obj/scene-plane.obj"); 
    mCube.Load("obj/cube.obj");
    mSphere.Load("obj/sphere.obj");
    mMonkey.Load("obj/monkey.obj");
    mKoliber.Load("obj/koliber.obj");
    playerObj.Load("obj/lego.obj"); 
    //playerBody.Load("obj/lego_body.obj"); 
    //playerLegR.Load("obj/lego_leg_r.obj"); 
    //playerLegL.Load("obj/lego_leg_l.obj"); 
    playerBody.Load("obj/cialko.obj"); 
    playerLegR.Load("obj/noga_prawa.obj"); 
    playerLegL.Load("obj/noga_lewa.obj");
    mTree.Load("obj/tree.obj");  
    mWall_1.Load("obj/wall_1.obj");
    mWall_2.Load("obj/wall_2.obj");
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
    meshes.push_back(playerObj); //7
    meshes.push_back(playerBody); //8
    meshes.push_back(playerLegR); //9
    meshes.push_back(playerLegL); //10
    meshes.push_back(mTree); //11
    meshes.push_back(mWall_1); //12
    meshes.push_back(mWall_2); //13

    //gdzie jest ziemia
    groundMeshPtr = &meshes[0];

    //RENDERING INSTANYCJNY

    //kwiaty
    flowerMatrices.clear();
    flowerColors.clear();
    for(int i = 0; i < FLOWER_COUNT; i++) {
        float x, z, y;
        bool found = false;
        
        // Próbuj znaleźć miejsce na podłodze (max 10 prób)
        for(int tries = 0; tries < 10; tries++) {
            x = (rand() % 2000 - 1000) / 10.0f; 
            z = (rand() % 2000 - 1000) / 10.0f;
            y = GetHeight(x, z);

            if (y > -900000.0f) { //jesli jest podloga
                found = true;
                break;
            }
        }

        if (found) {
            float a = glm::radians((float)(rand() % 360));
            float s = 0.3f + (rand() / (float)RAND_MAX) * 0.4f;

            glm::mat4 m = glm::mat4(1.0f);
            m = glm::translate(m, glm::vec3(x, y, z));
            m = glm::rotate(m, a, glm::vec3(0.0f, 1.0f, 0.0f));
            m = glm::scale(m, glm::vec3(s, s, s));
            flowerMatrices.push_back(m);
            flowerColors.push_back(glm::vec3(1.0f));
        }
    }

    // Przygotuj siatkę kwiatka (meshes[4]) do instancjonowania
    meshes[4].PrepareInstancing(flowerMatrices, flowerColors);

    //drzewa
    treeMatrices.clear();
    treeColors.clear();
    float minDist = 1.5f;
    for(int i = 0; i < TREE_COUNT; i++) {
    float x, z, y;
    bool foundPlace = false;
    
    // Próbuj znaleźć miejsce 
    for(int tries = 0; tries < 20; tries++) {
        x = (rand() % 2000 - 1000) / 10.0f; 
        z = (rand() % 2000 - 1000) / 10.0f;
        y = GetHeight(x, z);

        if (y > -900000.0f) { 
            // 1. BLOKADA PUNKTU STARTOWEGO
            glm::vec3 forbiddenPoint = glm::vec3(0.0f, 0.0f, 0.0f);
            float safeRadius = 3.0f; // Obszar wolny od drzew wokół punktu
            
            float distToForbidden = glm::distance(glm::vec2(x, z), glm::vec2(forbiddenPoint.x, forbiddenPoint.z));
            
            if (distToForbidden < safeRadius) {
                continue; // Losuj nową pozycję, to miejsce jest zakazane
            }

            // 2. SPRAWDZANIE DYSTANSU DO INNYCH DRZEW 
            bool tooClose = false;
            for(const auto& existingMat : treeMatrices) {
                glm::vec3 existingPos = glm::vec3(existingMat[3]);
                if(glm::distance(glm::vec2(x, z), glm::vec2(existingPos.x, existingPos.z)) < minDist) {
                    tooClose = true;
                    break;
                }
            }

            if (!tooClose) {
                foundPlace = true;
                break;
            }
        }
    }

    if (foundPlace) {
        float a = glm::radians((float)(rand() % 360));
        float s = 0.3f + (rand() / (float)RAND_MAX) * 0.4f;

        float r = 0.5f + (rand() % 50) / 100.0f;
        float g = 0.5f + (rand() % 50) / 100.0f;
        float b = 0.2f; 
        treeColors.push_back(glm::vec3(r, g, b));

        glm::mat4 m = glm::mat4(1.0f);
        m = glm::translate(m, glm::vec3(x, y, z));
        m = glm::rotate(m, a, glm::vec3(0.0f, 1.0f, 0.0f));
        m = glm::scale(m, glm::vec3(s, s, s));
        treeMatrices.push_back(m);
    }
    }

    // Przygotuj siatkę drzewa (meshes[11]) do instancjonowania
    meshes[11].PrepareInstancing(treeMatrices, treeColors);

    textures.push_back(0); // 0
    textures.push_back(LoadTexture("textures/grass.png")); 
    textures.push_back(LoadTexture("textures/metal.png")); 
    textures.push_back(LoadTexture("textures/brick.png")); //3
    textures.push_back(LoadTexture("textures/flower32bit.png"));
    textures.push_back(LoadTexture("textures/sand.png"));
    textures.push_back(LoadTexture("textures/lego.png")); //6
    textures.push_back(LoadTexture("textures/brick_normal.png")); //7
    textures.push_back(LoadTexture("textures/leaves.png")); //8
    textures.push_back(LoadTexture("textures/leaves_normal.png")); //9
    textures.push_back(0); //10
}

void BuildScene(std::vector<SceneObject>& scene, std::vector<CMesh>& meshes, std::vector<GLuint>& textures) {

    // --- OBIEKT 1: PODŁOGA Z HEIGHTMAPY ---
    /*
    SceneObject terrain;
    terrain.mesh = &meshes[6]; // Używamy mTerrain
    terrain.position = glm::vec3(0.0f, -22.0f, 0.0f); // Obniżamy nieco, by obiekty nie były pod ziemią
    terrain.rotation = glm::vec3(0.0f);
    terrain.scale = glm::vec3(1.0f); // Skala 1.0, bo LoadHeightmap już używa xzScale
    terrain.color = glm::vec3(1.0f, 0.0f, 1.0f); 
    terrain.idTexture = textures[5]; // Używamy piasku
    terrain.mat = {0.2f, 0.8f, 0.1f, 2.0f}; // Materiał matowy
    scene.push_back(terrain);*/

    // --- OBIEKT 1: NOWA PODLOGA ---
    SceneObject ground;
    ground.mesh = &meshes[0];
    ground.idTexture = textures[5];
    ground.position = glm::vec3(0.0f, 0.0f, 0.0f); 
    ground.scale = glm::vec3(4.0f, 1.0f, 4.0f);    //powiekszenie razy 4
    ground.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    ground.color = glm::vec3(1.0f);
    ground.mat = {0.2f, 0.8f, 0.1f, 32.0f};   // Parametry materiału
    scene.push_back(ground);

    // --- OBIEKT 2: Sześcian ---
    SceneObject box;
    box.mesh = &meshes[1];
    box.position = glm::vec3(3.0f, 0.5f, -2.0f);
    box.rotation = glm::vec3(0.0f);
    box.scale = glm::vec3(1.0f);
    box.color = glm::vec3(1.0f); 
    box.idTexture = textures[3]; // Cegły
    box.mat = {0.2f, 0.7f, 0.2f, 10.0f}; // Lekki połysk
    box.collider = new CAABBCollider(box.position, box.mesh->calculateHalfSizes() * box.scale, ColliderType::WALL); 
    box.idNormalMap = textures[7]; //normalmapa cegiel
    scene.push_back(box);

    // --- OBIEKT 3: Sfera (Najlepsza do testu Blinn-Phong!) ---
    // --- SFERA 1: BŁYSZCZĄCA (Shiny) ---
    SceneObject shinyBall;
    shinyBall.mesh = &meshes[2]; // Sfera
    shinyBall.position = glm::vec3(-5.0f, 1.0f, 0.0f);
    shinyBall.scale = glm::vec3(1.0f);
    shinyBall.color = glm::vec3(0.0f, 0.4f, 1.0f); // Niebieska
    shinyBall.idTexture = 0; // Bez tekstury, żeby lepiej widzieć blask
    // Materiał: wysoki specular (1.0) i mocne skupienie (128.0)
    shinyBall.mat = { 0.1f, 0.5f, 1.0f, 128.0f }; 
    shinyBall.collider = new CSphereCollider(shinyBall.position, 1.0f);
    scene.push_back(shinyBall);

    // --- SFERA 2: MATOWA (Matte) ---
    // Reaguje tylko na diffuse, nie ma "plamy" światła odbitego
    SceneObject matteBall;
    matteBall.mesh = &meshes[2]; // Ta sama siatka sfery
    matteBall.position = glm::vec3(5.0f, 1.0f, 0.0f);
    matteBall.scale = glm::vec3(1.0f);
    matteBall.color = glm::vec3(1.0f, 0.3f, 0.0f); // Pomarańczowa
    matteBall.idTexture = 0;
    // Materiał: specular ustawiony na 0.0 całkowicie wyłącza lśnienie
    matteBall.mat = { 0.2f, 0.8f, 0.0f, 1.0f }; 
    matteBall.collider = new CSphereCollider(matteBall.position, 1.0f);
    scene.push_back(matteBall);

    // --- OBIEKT 4: Małpka ---
    SceneObject suzanne;
    suzanne.mesh = &meshes[3]; 
    suzanne.position = glm::vec3(0.0f, 0.5f, 3.0f);
    suzanne.rotation = glm::vec3(0.0f, 3.14f, 0.0f); 
    suzanne.scale = glm::vec3(1.0f);
    suzanne.color = glm::vec3(1.0f, 0.8f, 0.1f); 
    suzanne.idTexture = 0;
    suzanne.mat = {0.2f, 0.8f, 0.5f, 32.0f}; // Złoty połysk
    suzanne.collider = new CSphereCollider(suzanne.position, 1.5f);
    scene.push_back(suzanne);
    /*
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
    scene.push_back(flower2);*/

    // --- OBIEKT 6: KOLIBER ---
    SceneObject koliber;
    koliber.mesh = &meshes[5]; 
    koliber.position = glm::vec3(0.0f, 5.0f, -5.0f); // Wyżej nad ziemią
    koliber.rotation = glm::vec3(0.0f, 1.57f, 0.0f);
    koliber.scale = glm::vec3(2.0f); 
    koliber.color = glm::vec3(0.1f, 0.5f, 0.4f); 
    koliber.idTexture = 0;
    koliber.mat = {0.2f, 0.8f, 1.0f, 128.0f};
    koliber.collider = new CSphereCollider(koliber.position, 1.5f); 
    scene.push_back(koliber);

    // --- OBIEKT 7: LUDZIK ---
    /*
    SceneObject playerObj;
    playerObj.mesh = &meshes[7];      
    playerObj.idTexture = textures[6]; 
    playerObj.scale = glm::vec3(0.5f); // Ludziki często są duże w OBJ, przeskaluj
    playerObj.color = glm::vec3(1.0f);
    playerObj.mat = {0.2f, 0.8f, 0.5f, 32.0f};
    // playerObj.position i rotation będziemy aktualizować co klatkę
    scene.push_back(playerObj);
    */

    // --- OBIEKT 8: NPC body ---
    SceneObject body;
    body.mesh = &meshes[8];
    //body.idTexture = textures[6]; 
    body.idTexture = 0;
    body.scale = glm::vec3(0.5f);
    body.color = glm::vec3(1.0f, 0.3f, 0.0f);
    body.position = glm::vec3(0.0f); 
    body.rotation = glm::vec3(0.0f);
    //body.position = glm::vec3(5.0f, -1.0f, 5.0f);
    //body.rotation = glm::vec3(0.0f, 1.57f, 0.0f);
    body.mat = {0.4f, 0.6f, 0.2f, 32.0f};
    scene.push_back(body);
    pIdx.body = scene.size() - 1;

    // --- OBIEKT 9: NPC prawa noga ---
    SceneObject legR;
    legR.mesh = &meshes[9];
    legR.idTexture = 0;
    legR.scale = glm::vec3(0.5f);
    legR.color = glm::vec3(0.0f, 0.4f, 1.0f);
    legR.position = glm::vec3(0.0f); // Ustaw startowo na zero
    legR.rotation = glm::vec3(0.0f);
    //legR.position = glm::vec3(5.0f, -1.0f, 5.0f);
    //legR.rotation = glm::vec3(0.0f, 1.57f, 0.0f);
    legR.mat = {0.4f, 0.6f, 0.2f, 32.0f};
    scene.push_back(legR);
    pIdx.legR = scene.size() - 1;

    // --- OBIEKT 10: NPC lewa noga ---
    SceneObject legL;
    legL.mesh = &meshes[10];
    legL.idTexture = 0;
    legL.scale = glm::vec3(0.5f);
    legL.color = glm::vec3(1.0f, 0.8f, 0.1f); 
    legL.position = glm::vec3(0.0f);
    legL.rotation = glm::vec3(0.0f);
    //legL.position = glm::vec3(5.0f, -1.0f, 5.0f);
    //legL.rotation = glm::vec3(0.0f, 1.57f, 0.0f);
    legL.mat = {0.4f, 0.6f, 0.2f, 32.0f};
    scene.push_back(legL);
    pIdx.legL = scene.size() - 1;

    //int playerSceneIndex = scene.size() - 1;

    // --- OBIEKT 11: Sciana poziom ---
    SceneObject wall_1;
    wall_1.mesh = &meshes[12];
    wall_1.position = glm::vec3(5.0f, -1.0f, 5.0f);
    wall_1.rotation = glm::vec3(0.0f);
    wall_1.scale = glm::vec3(1.0f);
    wall_1.color = glm::vec3(1.0f); 
    wall_1.idTexture = textures[8]; // liscie
    wall_1.mat = {0.2f, 0.7f, 0.2f, 10.0f}; // Lekki połysk
    wall_1.collider = new CAABBCollider(wall_1.position, wall_1.mesh->calculateHalfSizes() * wall_1.scale, ColliderType::WALL);
    wall_1.idNormalMap = textures[9]; //normalmapa lisci
    scene.push_back(wall_1);

    // --- OBIEKT 12: Sciana pion ---
    SceneObject wall_2;
    wall_2.mesh = &meshes[13];
    wall_2.position = glm::vec3(8.0f, -1.0f, 6.0f);
    wall_2.rotation = glm::vec3(0.0f);
    wall_2.scale = glm::vec3(1.0f);
    wall_2.color = glm::vec3(1.0f); 
    wall_2.idTexture = textures[8]; // liscie
    wall_2.mat = {0.2f, 0.7f, 0.2f, 10.0f}; // Lekki połysk
    wall_2.collider = new CAABBCollider(wall_2.position, wall_2.mesh->calculateHalfSizes() * wall_2.scale, ColliderType::COLLECTIBLE);
    wall_2.idNormalMap = textures[9]; //normalmapa lisci
    scene.push_back(wall_2);
}

void LoadHeightmap(const char* filename, std::vector<TerrainVertex>& vertices, std::vector<unsigned int>& indices) {
    int width, height, channels;
    // Wczytujemy obrazek jako czarno-biały (1 kanał - STBI_grey)
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 1);
    
    if (!data) {
        printf("Błąd wczytywania heightmapy: %s\n", filename);
        return;
    }

    hmWidth = width; 
    hmHeight = height;
    globalHeights.clear();

    // 1. Generowanie wierzchołków
    for (int z = 0; z < height; z++) {
        for (int x = 0; x < width; x++) {
            float y = data[z * width + x] * hmYScale;
            globalHeights.push_back(y);
            
            TerrainVertex v;
            // Centrujemy teren, odejmując połowę szerokości
            v.position = glm::vec3((x - width/2.0f) * hmXZScale, y, (z - height/2.0f) * hmXZScale);
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
//instancjonowanie 
void CMesh::PrepareInstancing(const std::vector<glm::mat4>& matrices, const std::vector<glm::vec3>& colors) {
    this->instanceCount = (int)matrices.size();
    this->isInstanced = true;

    glBindVertexArray(idVAO);
    
    //dla macierzy
    glGenBuffers(1, &idVBO_instance);
    glBindBuffer(GL_ARRAY_BUFFER, idVBO_instance);
    glBufferData(GL_ARRAY_BUFFER, matrices.size() * sizeof(glm::mat4), &matrices[0][0][0], GL_STATIC_DRAW);

    // Rejestracja macierzy w slotach 3, 4, 5, 6
    for (int i = 0; i < 4; i++) {
        glEnableVertexAttribArray(3 + i);
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * i));
        glVertexAttribDivisor(3 + i, 1); 
    }

    //dla kolorow
    GLuint vboColor;
    glGenBuffers(1, &vboColor);
    glBindBuffer(GL_ARRAY_BUFFER, vboColor);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), &colors[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(7); // Atrybut nr 7
    glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glVertexAttribDivisor(7, 1); 

    glBindVertexArray(0);
}

glm::vec3 CMesh::calculateHalfSizes() {
    if (vertices.empty()) return glm::vec3(0.0f);
    
    glm::vec3 minV = vertices[0];
    glm::vec3 maxV = vertices[0];
    
    for (const auto& v : vertices) {
        minV = glm::min(minV, v);
        maxV = glm::max(maxV, v);
    }
    // Połowa różnicy między max a min to nasze HalfSizes
    return (maxV - minV) * 0.5f;
}