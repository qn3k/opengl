#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// -------------------------------------------------------
// Zmienne globalne
// -------------------------------------------------------
int windowWidth = 800;
int windowHeight = 600;
const char *windowTitle = "Animacja: Monkey & Cones";

// -------------------------------------------------------
// INCLUDE NAGŁÓWKÓW POMOCNICZYCH
// -------------------------------------------------------
#include "utilities.hpp" 
#include "objloader.hpp" 

// -------------------------------------------------------
// KLASA: CMesh
// -------------------------------------------------------
class CMesh {
public:
    GLuint idVAO;
    GLuint idVBO_pos;
    GLuint idVBO_norm;
    int vertexCount;

    CMesh() : idVAO(0), vertexCount(0) {}

    bool Load(const char* path) {
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec3> normals;

        if (!loadOBJ(path, vertices, uvs, normals)) {
            printf("Nie udalo sie wczytac modelu: %s\n", path);
            return false;
        }

        vertexCount = vertices.size();

        glGenVertexArrays(1, &idVAO);
        glBindVertexArray(idVAO);

        // Pozycje
        glGenBuffers(1, &idVBO_pos);
        glBindBuffer(GL_ARRAY_BUFFER, idVBO_pos);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);

        // Normalne
        glGenBuffers(1, &idVBO_norm);
        glBindBuffer(GL_ARRAY_BUFFER, idVBO_norm);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
        printf("Zaladowano model: %s (wierzcholkow: %d)\n", path, vertexCount);
        return true;
    }

    void Draw() {
        if (idVAO == 0) return;
        glBindVertexArray(idVAO);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glBindVertexArray(0);
    }

    void Release() {
        glDeleteBuffers(1, &idVBO_pos);
        glDeleteBuffers(1, &idVBO_norm);
        glDeleteVertexArrays(1, &idVAO);
    }
};

// -------------------------------------------------------
// STRUKTURA: SceneObject
// -------------------------------------------------------
struct SceneObject {
    CMesh* mesh;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    glm::vec3 colorBottom; 
    glm::vec3 colorTop;
};

// -------------------------------------------------------
// Zmienne globalne aplikacji
// -------------------------------------------------------
GLuint idProgram;

// Kamera
float cameraDist = 10.0f;
float cameraAngleX = 0.5f; 
float cameraAngleY = 0.0f;

// Kontenery
std::vector<CMesh> meshes;      
std::vector<SceneObject> scene; 

glm::mat4 matProj;
glm::mat4 matView;

// -------------------------------------------------------
// Inicjalizacja Sceny
// -------------------------------------------------------
void Initialize()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); 
    glEnable(GL_DEPTH_TEST);

    // 1. Szadery
    idProgram = glCreateProgram();
    glAttachShader(idProgram, LoadShader(GL_VERTEX_SHADER, "vertex.glsl"));
    glAttachShader(idProgram, LoadShader(GL_FRAGMENT_SHADER, "fragment.glsl"));
    LinkAndValidateProgram(idProgram);

    // 2. Wczytywanie modeli 
    CMesh meshMonkey, meshCone;
    if(!meshMonkey.Load("monkey.obj")) printf("Błąd monkey.obj\n");
    if(!meshCone.Load("cone.obj"))     printf("Błąd cone.obj\n");

    meshes.push_back(meshMonkey); // index 0
    meshes.push_back(meshCone);   // index 1

    scene.clear();

    // --- MAŁPKA ---
    SceneObject monkey;
    monkey.mesh = &meshes[0]; 
    monkey.position = glm::vec3(0.0f, 0.0f, 0.0f);
    monkey.rotation = glm::vec3(0.0f);
    monkey.scale    = glm::vec3(1.0f); 
    // Gradient od ciemnego złota do jasnego żółtego
    monkey.colorBottom = glm::vec3(0.8f, 0.4f, 0.0f); 
    monkey.colorTop    = glm::vec3(1.0f, 0.9f, 0.4f);
    scene.push_back(monkey);

    // --- STOŻKI ---
    
    struct ColorPair { glm::vec3 c1; glm::vec3 c2; };
    ColorPair coneColors[4] = {
        { glm::vec3(0.0f, 0.3f, 0.0f), glm::vec3(0.2f, 1.0f, 0.2f) }, // Ciemna zieleń -> Jasna zieleń
        { glm::vec3(0.5f, 0.0f, 0.0f), glm::vec3(1.0f, 0.4f, 0.4f) }, // Ciemna czerwień -> Róż
        { glm::vec3(0.0f, 0.0f, 0.5f), glm::vec3(0.2f, 0.6f, 1.0f) }, // Granat -> Błękit
        { glm::vec3(0.3f, 0.0f, 0.3f), glm::vec3(0.9f, 0.3f, 0.9f) }  // Ciemny fiolet -> Jasny fiolet
    };

    for(int i = 0; i < 4; i++) {
        SceneObject cone;
        cone.mesh = &meshes[1]; 
        cone.position = glm::vec3(0.0f); 
        cone.rotation = glm::vec3(0.0f);
        cone.scale    = glm::vec3(0.4f, 2.5f, 0.4f); 
        
        // Przypisanie gradientu
        cone.colorBottom = coneColors[i].c1;
        cone.colorTop    = coneColors[i].c2;
        
        scene.push_back(cone);
    }

    if (windowHeight != 0)
        matProj = glm::perspective(glm::radians(60.0f), windowWidth/(float)windowHeight, 0.1f, 100.0f);
}

// -------------------------------------------------------
// Renderowanie Sceny
// -------------------------------------------------------
void DisplayScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(idProgram);

    // 1. Kamera
    matView = glm::mat4(1.0);
    matView = glm::translate(matView, glm::vec3(0.0f, 0.0f, -cameraDist));
    matView = glm::rotate(matView, cameraAngleX, glm::vec3(1.0f, 0.0f, 0.0f));
    matView = glm::rotate(matView, cameraAngleY, glm::vec3(0.0f, 1.0f, 0.0f));
    
    glUniformMatrix4fv(glGetUniformLocation(idProgram, "matProj"), 1, GL_FALSE, glm::value_ptr(matProj));
    glUniformMatrix4fv(glGetUniformLocation(idProgram, "matView"), 1, GL_FALSE, glm::value_ptr(matView));

    float time = glfwGetTime();

    // --- ANIMACJA ---

    // 1. Małpka (indeks 0)
    // Obraca się w jednej osi (Y) np. w prawo
    if(scene.size() > 0) {
        scene[0].rotation.y = time * 2.0f;       // Szybki obrót własny
        scene[0].rotation.x = sin(time) * 0.2f;  // Lekkie "kiwanie" głową góra/dół
    }

    // 2. Stożki (indeksy 1-4)
    float radius = 4.0f; // Promień koła
    float orbitSpeed = 1.0f; 

    for(int i = 1; i < scene.size(); i++) {
        // Przesunięcie każdego stożka o 90 stopni (PI/2)
        // (i-1) daje nam mnożnik 0, 1, 2, 3
        float offset = (i - 1) * (glm::half_pi<float>());
        
        // Kąt orbity: (-time) zapewnia obrót w stronę przeciwną niż małpka
        float angle = (-time * orbitSpeed) + offset;

        // Pozycja na płaszczyźnie XZ (Y = 0, czyli płasko)
        scene[i].position.x = radius * cos(angle);
        scene[i].position.z = radius * sin(angle);
        scene[i].position.y = 0.0f; // Poziom małpki

        // --- ROTACJA STOŻKA ---
        // Obrót o 90 stopni wokół X kładzie go na płasko.
        float rotX = glm::radians(90.0f);

        // 2. Obracamy go wokół Y, aby celował w środek (0,0,0).
        float rotY = -angle + glm::radians(90.0f);

        // Przypisanie do obiektu
        scene[i].rotation.x = rotX;
        scene[i].rotation.y = rotY;
        scene[i].rotation.z = 0.0f;
    }

    // --- RYSOWANIE ---
    for (const auto& obj : scene)
    {
        if(!obj.mesh) continue;

        glm::mat4 matModel = glm::mat4(1.0);
        matModel = glm::translate(matModel, obj.position);
        
        // Kolejność rotacji (Y potem X jest bezpieczniejsza dla orbitowania obiektów leżących)
        matModel = glm::rotate(matModel, obj.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        matModel = glm::rotate(matModel, obj.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        matModel = glm::rotate(matModel, obj.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        
        matModel = glm::scale(matModel, obj.scale);

        glUniformMatrix4fv(glGetUniformLocation(idProgram, "matModel"), 1, GL_FALSE, glm::value_ptr(matModel));

        glUniform3fv(glGetUniformLocation(idProgram, "uColor1"), 1, glm::value_ptr(obj.colorBottom));
        glUniform3fv(glGetUniformLocation(idProgram, "uColor2"), 1, glm::value_ptr(obj.colorTop));

        obj.mesh->Draw();
    }

    glUseProgram(0);
}

// -------------------------------------------------------
// Callbacki (bez zmian merytorycznych, obsługa kamery)
// -------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
    matProj = glm::perspective(glm::radians(60.0f), width/(float)height, 0.1f, 100.0f);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch(key) {
            case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
            case GLFW_KEY_W: cameraAngleX += 0.05f; break;
            case GLFW_KEY_S: cameraAngleX -= 0.05f; break;
            case GLFW_KEY_A: cameraAngleY -= 0.05f; break;
            case GLFW_KEY_D: cameraAngleY += 0.05f; break;
            case GLFW_KEY_EQUAL:      
            case GLFW_KEY_KP_ADD: cameraDist -= 0.5f; if(cameraDist < 2.0f) cameraDist = 2.0f; break;
            case GLFW_KEY_MINUS:      
            case GLFW_KEY_KP_SUBTRACT: cameraDist += 0.5f; break;
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    cameraDist -= yoffset;
    if (cameraDist < 2.0f) cameraDist = 2.0f;
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {}

// -------------------------------------------------------
// MAIN
// -------------------------------------------------------
int main(int argc, char *argv[])
{
    GLFWwindow* window = nullptr;
    Initialize_GLFW(window);
    Initialize();

    while (!glfwWindowShouldClose(window))
    {
        DisplayScene();
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glDeleteProgram(idProgram);
    for(auto& m : meshes) m.Release();

    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}