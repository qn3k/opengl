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
int windowWidth = 800;
int windowHeight = 600;
const char* windowTitle = "Skybox v1";
glm::mat4 matProj;
glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f); // Domyślnie białe

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "utilities.hpp" 
#include "objloader.hpp" 
#include "SceneManager.hpp"
#include "Skybox.h"

// --- USTAWIENIA GLOBALNE ---
bool useLighting = true;
bool useBlinnPhong = false; 
bool animateLight = true; // Flaga animacji
float lightOrbitSpeed = 1.0f; // Prędkość obrotu
bool showLightSource = true; // Czy pokazywać "żarówkę"
bool isPointLight = true; // true = Punktowe, false = Kierunkowe
glm::vec3 dirLightDirection = glm::vec3(-0.2f, -1.0f, -0.3f); // Światło padające z góry pod skosem
Skybox* skybox1 = nullptr;
Skybox* skybox2 = nullptr;
int currentSkybox = 0;

// --- STRUKTURA ŚWIATŁA ---
struct PointLight {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
};

PointLight lights[4];
int activeLightsCount = 1; // Startujemy z jednym światłem


// Zmienne
GLuint idProgram;
std::vector<CMesh> meshes;
std::vector<SceneObject> scene;
std::vector<GLuint> textures;

void Initialize() {

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); 
    glEnable(GL_DEPTH_TEST);
    
    // Włączenie blendowania dla przezroczystości tekstur kwiatów
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 1. Szadery
    idProgram = glCreateProgram();
    glAttachShader(idProgram, LoadShader(GL_VERTEX_SHADER, "shaders/vertex.glsl"));
    glAttachShader(idProgram, LoadShader(GL_FRAGMENT_SHADER, "shaders/fragment.glsl"));
    LinkAndValidateProgram(idProgram);

    //Przeniesienie ladowania obiektow do zewnetrznych funkcji
    InitializeResources(meshes, textures);
    BuildScene(scene, meshes, textures);

    // Konfiguracja Skyboxa
    std::vector<std::string> faces1 = {"skybox1/posx.jpg","skybox1/negx.jpg","skybox1/posy.jpg","skybox1/negy.jpg","skybox1/posz.jpg","skybox1/negz.jpg"};
    std::vector<std::string> faces2 = {"skybox2/posx.jpg","skybox2/negx.jpg","skybox2/posy.jpg","skybox2/negy.jpg","skybox2/posz.jpg","skybox2/negz.jpg"};

    skybox1 = new Skybox(faces1, "shaders/skybox-vertex.glsl", "shaders/skybox-fragment.glsl");
    skybox2 = new Skybox(faces2, "shaders/skybox-vertex.glsl", "shaders/skybox-fragment.glsl");
    
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

    // Wykorzystanie funkcji z utilities.hpp
    glm::mat4 matView = UpdateViewMatrix();
    //glm::vec3 lightPos = glm::vec3(2.0f, 4.0f, 2.0f);
    glm::vec3 camPos = ExtractCameraPos(matView);  

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

    ImGui::Text("\nWybierz Skybox:");
    ImGui::RadioButton("Jeden", &currentSkybox, 0);
    ImGui::RadioButton("Dwa", &currentSkybox, 1);

    ImGui::End();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);

    glm::mat4 viewStatic = glm::mat4(glm::mat3(matView));

    if (currentSkybox == 0 && skybox1) {
        skybox1->draw(matProj, viewStatic, 40.0f);
    } 
    else if (currentSkybox == 1 && skybox2) {
        skybox2->draw(matProj, viewStatic, 40.0f);
    }

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

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

    glActiveTexture(GL_TEXTURE1);
    if (currentSkybox == 0) glBindTexture(GL_TEXTURE_CUBE_MAP, skybox1->getTextureID());
    else glBindTexture(GL_TEXTURE_CUBE_MAP, skybox2->getTextureID());
    glUniform1i(glGetUniformLocation(idProgram, "tex_skybox"), 1);

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

        // Jeśli to koliber, ustaw siłę odbicia
        float rFact = 0.0f;
        if (obj.mesh == &meshes[5]) rFact = 0.6f; // Koliber odbija w 60%
        glUniform1f(glGetUniformLocation(idProgram, "reflectionFactor"), rFact);

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
    delete skybox1;
    delete skybox2;
    glfwTerminate();
    return 0;
}