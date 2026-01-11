#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Musimy zadeklarować matProj tutaj, bo utilities.hpp go używa
glm::mat4 matProj;
glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f); // Domyślnie białe

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "utilities.hpp" 
#include "objloader.hpp" 

// --- USTAWIENIA GLOBALNE ---
bool useLighting = true;
bool useBlinnPhong = false; 
bool animateLight = true; // Flaga animacji
float lightOrbitSpeed = 1.0f; // Prędkość obrotu
bool showLightSource = true; // Czy pokazywać "żarówkę"
bool isPointLight = true; // true = Punktowe, false = Kierunkowe
glm::vec3 dirLightDirection = glm::vec3(-0.2f, -1.0f, -0.3f); // Światło padające z góry pod skosem

// --- STRUKTURA ŚWIATŁA ---
struct PointLight {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
};

PointLight lights[4];
int activeLightsCount = 1; // Startujemy z jednym światłem

// --- STRUKTURA MATERIAŁU I OBIEKTU ---
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

    bool Load(const char* path) {
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
        return true;
    }

    void Draw() { if (idVAO) { glBindVertexArray(idVAO); glDrawArrays(GL_TRIANGLES, 0, vertexCount); } }
    void Release() { glDeleteBuffers(1, &idVBO_pos); glDeleteBuffers(1, &idVBO_norm); glDeleteBuffers(1, &idVBO_uv); glDeleteVertexArrays(1, &idVAO); }

    //Kwiatki
    static CMesh CreateFlowerMesh() {
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
        
        // Uproszczone normalne
        // Dwa pierwsze kwadraty leżą w płaszczyźnie XY (normalna 0,0,1)
        // Dwa drugie kwadraty leżą w płaszczyźnie YZ (normalna 1,0,0)
        GLfloat normals[12*3];
        for(int i=0; i<6; ++i) { normals[i*3] = 0.0f; normals[i*3+1] = 0.0f; normals[i*3+2] = 1.0f; } // Normalna wzdluz Z
        for(int i=6; i<12; ++i) { normals[i*3] = 1.0f; normals[i*3+1] = 0.0f; normals[i*3+2] = 0.0f; } // Normalna wzdluz X

        mesh.vertexCount = 12;

        // Generowanie VAO
        glGenVertexArrays(1, &mesh.idVAO);
        glBindVertexArray(mesh.idVAO);

        // VBO: Pozycje (location = 0)
        glGenBuffers(1, &mesh.idVBO_pos);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.idVBO_pos);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);

        // VBO: Normalne (location = 2)
        glGenBuffers(1, &mesh.idVBO_norm);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.idVBO_norm);
        glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(2);
        
        // VBO: Wspolrzedne UV (location = 1)
        glGenBuffers(1, &mesh.idVBO_uv);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.idVBO_uv);
        glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
        printf("Utworzono siatke kwiatu (dwa kwadraty).\n");
        return mesh;
    }

};

struct SceneObject {
    CMesh* mesh;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    glm::vec3 color;
    GLuint idTexture; // 0 jeśli brak tekstury
    Material mat;     // Parametry oświetlenia
};

// Zmienne
GLuint idProgram;
std::vector<CMesh> meshes;
std::vector<SceneObject> scene;
std::vector<GLuint> textures; //Kontener na wczytane tekstury

GLuint LoadTexture(const char* path) {
    GLuint textureID;
    int tex_width, tex_height, tex_n;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *tex_data = stbi_load(path, &tex_width, &tex_height, &tex_n, 0);
    if (tex_data == NULL) return 0;

    GLenum format = (tex_n == 4) ? GL_RGBA : GL_RGB;

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, tex_width, tex_height, 0, format, GL_UNSIGNED_BYTE, tex_data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(tex_data);
    return textureID;
}

void Initialize() {

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); 
    glEnable(GL_DEPTH_TEST);
    
    // Włączenie blendowania dla przezroczystości tekstur kwiatów
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 1. Szadery
    idProgram = glCreateProgram();
    glAttachShader(idProgram, LoadShader(GL_VERTEX_SHADER, "vertex.glsl"));
    glAttachShader(idProgram, LoadShader(GL_FRAGMENT_SHADER, "fragment.glsl"));
    LinkAndValidateProgram(idProgram);

    // 2. Wczytywanie modeli do pamięci
    CMesh meshGround, meshCube, meshSphere, meshMonkey, meshFlower;
    
    meshGround.Load("cube.obj"); 
    meshCube.Load("cube.obj");
    meshSphere.Load("sphere.obj");
    meshMonkey.Load("monkey.obj");
    meshFlower = CMesh::CreateFlowerMesh(); // Tworzenie siatki kwiatka (krzyż)

    // Dodawanie do kontenera w stałej kolejności (ważne dla indeksów!)
    meshes.push_back(meshGround); // indeks 0
    meshes.push_back(meshCube);   // indeks 1
    meshes.push_back(meshSphere); // indeks 2
    meshes.push_back(meshMonkey); // indeks 3
    meshes.push_back(meshFlower); // indeks 4

    // 3. Wczytywanie tekstur
    textures.push_back(0);                             // indeks 0: brak
    textures.push_back(LoadTexture("grass.png"));      // indeks 1
    textures.push_back(LoadTexture("metal.png"));      // indeks 2
    textures.push_back(LoadTexture("brick.png"));      // indeks 3
    textures.push_back(LoadTexture("flower32bit.png"));// indeks 4

    // 4. Tworzenie obiektów na scenie z parametrami MATERIAŁU
    // Parametry: {Ambient, Diffuse, Specular, Shininess}

    // --- OBIEKT 1: Podłoże ---
    SceneObject ground;
    ground.mesh = &meshes[0]; 
    ground.position = glm::vec3(0.0f, -1.0f, 0.0f);
    ground.rotation = glm::vec3(0.0f);
    ground.scale = glm::vec3(8.0f, 0.1f, 8.0f); 
    ground.color = glm::vec3(1.0f); 
    ground.idTexture = textures[1]; // Trawa
    ground.mat = {0.2f, 0.8f, 0.0f, 1.0f}; // Matowa trawa (brak specular)
    scene.push_back(ground);

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

    matProj = glm::perspective(glm::radians(80.0f), windowWidth/(float)windowHeight, 0.1f, 50.0f);
}

void SetupLights() {
    // Światło 1: Białe (krążące - to co już masz)
    lights[0] = { glm::vec3(0, 3, 0), glm::vec3(1.0, 1.0, 1.0), 1.5f };
    // Światło 2: Czerwone (stałe)
    lights[1] = { glm::vec3(-4, 2, -4), glm::vec3(1.0, 0.0, 0.0), 1.2f };
    // Światło 3: Zielone (stałe)
    lights[2] = { glm::vec3(4, 2, -4), glm::vec3(0.0, 1.0, 0.0), 1.2f };
    // Światło 4: Niebieskie (stałe)
    lights[3] = { glm::vec3(0, 2, 4), glm::vec3(0.0, 0.0, 1.0), 1.2f };
}

void DisplayScene() {
    // --- START IMGUI FRAME ---
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // --- OKNO KONTROLNE ---
    ImGui::Begin("Lighting Controller");

    if (ImGui::CollapsingHeader("Global Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Use Lighting (F1/F2)", &useLighting);
        ImGui::Checkbox("Blinn-Phong Model (P/B)", &useBlinnPhong);
        ImGui::Checkbox("Animate Lights (M)", &animateLight);
        ImGui::Checkbox("Show Light Sources (G)", &showLightSource);
        ImGui::SliderFloat("Orbit Speed", &lightOrbitSpeed, 0.0f, 5.0f);
    }

    if (ImGui::CollapsingHeader("Light Type", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::RadioButton("Point Light", isPointLight)) isPointLight = true;
        ImGui::SameLine();
        if (ImGui::RadioButton("Directional Light", !isPointLight)) isPointLight = false;
        
        if (!isPointLight) {
            ImGui::SliderFloat3("Direction", glm::value_ptr(dirLightDirection), -1.0f, 1.0f);
        }
    }

    if (ImGui::CollapsingHeader("Multi-Light (N)", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderInt("Active Lights", &activeLightsCount, 1, 4);
        
        for (int i = 0; i < activeLightsCount; i++) {
            ImGui::PushID(i);
            char buf[32];
            sprintf(buf, "Light %d", i + 1);
            if (ImGui::TreeNode(buf)) {
                ImGui::ColorEdit3("Color", glm::value_ptr(lights[i].color));
                ImGui::SliderFloat("Intensity", &lights[i].intensity, 0.0f, 5.0f);
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
    }

    ImGui::End();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(idProgram);

    // 1. Najpierw aktualizujemy pozycje (Animacja)
    float time = (float)glfwGetTime();
    for (int i = 0; i < 4; i++) {
        if (animateLight) {
            float angle = time * lightOrbitSpeed + (i * 1.57f);
            float radius = 4.0f + i;
            lights[i].position = glm::vec3(sin(angle) * radius, 2.5f, cos(angle) * radius);
        }
    }

    static float lightAngle = 0.0f;
    if (animateLight) lightAngle = (float)glfwGetTime() * lightOrbitSpeed;
    glm::vec3 currentLightPos = glm::vec3(sin(lightAngle) * 5.0f, 3.0f, cos(lightAngle) * 5.0f);    

    // Wykorzystanie funkcji z utilities.hpp
    glm::mat4 matView = UpdateViewMatrix();
    //glm::vec3 lightPos = glm::vec3(2.0f, 4.0f, 2.0f);
    glm::vec3 camPos = ExtractCameraPos(matView);

    glUniformMatrix4fv(glGetUniformLocation(idProgram, "matProj"), 1, GL_FALSE, glm::value_ptr(matProj));
    glUniformMatrix4fv(glGetUniformLocation(idProgram, "matView"), 1, GL_FALSE, glm::value_ptr(matView));
    glUniform3fv(glGetUniformLocation(idProgram, "lightPos"), 1, glm::value_ptr(currentLightPos));
    glUniform3fv(glGetUniformLocation(idProgram, "lightColor"), 1, glm::value_ptr(lightColor)); 
    glUniform3fv(glGetUniformLocation(idProgram, "viewPos"), 1, glm::value_ptr(camPos));
    glUniform1i(glGetUniformLocation(idProgram, "useLighting"), useLighting);
    glUniform1i(glGetUniformLocation(idProgram, "useBlinnPhong"), useBlinnPhong);
    glUniform1i(glGetUniformLocation(idProgram, "bIsLightSource"), false);
    glUniform3fv(glGetUniformLocation(idProgram, "dirLightDirection"), 1, glm::value_ptr(dirLightDirection));
    glUniform1i(glGetUniformLocation(idProgram, "isPointLight"), isPointLight);
    glUniform1i(glGetUniformLocation(idProgram, "activeLightsCount"), activeLightsCount);

    // PRZESYŁANIE TABLICY ŚWIATEŁ PUNKTOWYCH
    for (int i = 0; i < 4; i++) {
        std::string base = "lights[" + std::to_string(i) + "]";
        
        // Ustawiamy kolory świateł na podstawie globalnego lightColor, 
        // aby klawisze 1-4 nadal działały na główne światło
        if(i == 0) lights[i].color = lightColor; 

        glUniform3fv(glGetUniformLocation(idProgram, (base + ".position").c_str()), 1, glm::value_ptr(lights[i].position));
        glUniform3fv(glGetUniformLocation(idProgram, (base + ".color").c_str()), 1, glm::value_ptr(lights[i].color));
        glUniform1f(glGetUniformLocation(idProgram, (base + ".intensity").c_str()), lights[i].intensity);
    }



    GLint loc_bUseTexture = glGetUniformLocation(idProgram, "bUseTexture");

    for (const auto& obj : scene) {
        // Macierz modelu
        glm::mat4 matModel = glm::mat4(1.0);
        matModel = glm::translate(matModel, obj.position);
        matModel = glm::rotate(matModel, obj.rotation.y, glm::vec3(0,1,0));
        matModel = glm::scale(matModel, obj.scale);
        
        glUniformMatrix4fv(glGetUniformLocation(idProgram, "matModel"), 1, GL_FALSE, glm::value_ptr(matModel));
        glUniform3fv(glGetUniformLocation(idProgram, "uColor"), 1, glm::value_ptr(obj.color));

        // Przesyłanie materiału
        glUniform1f(glGetUniformLocation(idProgram, "material.ambient"), obj.mat.ambient);
        glUniform1f(glGetUniformLocation(idProgram, "material.diffuse"), obj.mat.diffuse);
        glUniform1f(glGetUniformLocation(idProgram, "material.specular"), obj.mat.specular);
        glUniform1f(glGetUniformLocation(idProgram, "material.shininess"), obj.mat.shininess);

        // Obsługa tekstury
        if (obj.idTexture != 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, obj.idTexture);
            glUniform1i(glGetUniformLocation(idProgram, "uTextureSampler"), 0);
            glUniform1i(loc_bUseTexture, 1);
        } else {
            glUniform1i(loc_bUseTexture, 0);
        }

        obj.mesh->Draw();
    }

    //RYSOWANIE "ŻARÓWKI" (klawisz G)
    if (showLightSource && isPointLight) {
        glUniform1i(glGetUniformLocation(idProgram, "bIsLightSource"), true);
        
        for (int i = 0; i < activeLightsCount; i++) {
            glm::mat4 matLight = glm::translate(glm::mat4(1.0), lights[i].position);
            matLight = glm::scale(matLight, glm::vec3(0.15f));
            
            // Przesyłamy kolor konkretnej żarówki do shadera, żeby sferka świeciła na swój kolor
            glUniform3fv(glGetUniformLocation(idProgram, "lightColor"), 1, glm::value_ptr(lights[i].color));
            glUniformMatrix4fv(glGetUniformLocation(idProgram, "matModel"), 1, GL_FALSE, glm::value_ptr(matLight));
            
            meshes[2].Draw(); // Rysuj sferę
        }
        glUniform1i(glGetUniformLocation(idProgram, "bIsLightSource"), false);
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// Nadpisujemy key_callback
void my_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    key_callback(window, key, scancode, action, mods);

    if (action == GLFW_PRESS) {
        
        // F1: Renderowanie BEZ oświetlenia (czysty kolor/tekstura)
        if (key == GLFW_KEY_F1) {
            useLighting = false;
            printf("Tryb renderingu: BEZ OSWIETLENIA (Unlit)\n");
        }

        // F2: Renderowanie Z oświetleniem (Phong / Blinn-Phong)
        if (key == GLFW_KEY_F2) {
            useLighting = true;
            printf("Tryb renderingu: Z OSWIETLENIEM (Lit)\n");
        }

        // Pozostałe klawisze 
        if (key == GLFW_KEY_P) { useBlinnPhong = false; printf("Model: Phong\n"); }
        if (key == GLFW_KEY_B) { useBlinnPhong = true; printf("Model: Blinn-Phong\n"); }
        //Klawisz animacji
        if (key == GLFW_KEY_M && action == GLFW_PRESS) {
            animateLight = !animateLight;
            printf("Animacja swiatla: %s\n", animateLight ? "ON" : "OFF");
        }
        if (key == GLFW_KEY_G && action == GLFW_PRESS) {
            showLightSource = !showLightSource;
            printf("Widocznosc zrodla swiatla (G): %s\n", showLightSource ? "ON" : "OFF");
        }
        if (key == GLFW_KEY_H && action == GLFW_PRESS) {
            isPointLight = !isPointLight;
            printf("Typ oswietlenia (H): %s\n", isPointLight ? "PUNKTOWE" : "KIERUNKOWE");
        }

        if (key == GLFW_KEY_N && action == GLFW_PRESS) {
            activeLightsCount++;
            if (activeLightsCount > 4) activeLightsCount = 1;
            printf("Liczba aktywnych swiatel: %d\n", activeLightsCount);
        }
        
        // Kolory światła
        if (key == GLFW_KEY_1) { lightColor = glm::vec3(1.0, 1.0, 1.0); } // White
        if (key == GLFW_KEY_2) { lightColor = glm::vec3(1.0, 0.0, 0.0); } // Red
        if (key == GLFW_KEY_3) { lightColor = glm::vec3(0.0, 1.0, 0.0); } // Green
        if (key == GLFW_KEY_4) { lightColor = glm::vec3(0.0, 0.0, 1.0); } // Blue
    }
}

int main() {
    if (!glfwInit()) return -1;
    glfwSetErrorCallback(error_callback);

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, NULL, NULL);
    if (!window) return -1;

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Style
    ImGui::StyleColorsDark();

    // Rejestracja callbacków z utilities.hpp
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, my_key_callback);

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    Initialize();
    SetupLights();
    
    while (!glfwWindowShouldClose(window)) {
        DisplayScene();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    for(auto& m : meshes) m.Release();
    glfwTerminate();
    return 0;
}