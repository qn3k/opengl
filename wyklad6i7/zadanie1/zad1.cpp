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

#include "utilities.hpp" 
#include "objloader.hpp" 

// --- USTAWIENIA GLOBALNE ---
bool useLighting = true;
bool useBlinnPhong = false; 

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
};

struct SceneObject {
    CMesh* mesh;
    glm::vec3 position;
    glm::vec3 color;
    Material mat;
    GLuint textureID = 0;
};

// Zmienne
GLuint idProgram;
std::vector<CMesh> meshes;
std::vector<SceneObject> scene;

void Initialize() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    idProgram = glCreateProgram();
    glAttachShader(idProgram, LoadShader(GL_VERTEX_SHADER, "vertex.glsl"));
    glAttachShader(idProgram, LoadShader(GL_FRAGMENT_SHADER, "fragment.glsl"));
    LinkAndValidateProgram(idProgram);

    // Załadowanie modeli
    meshes.resize(3);
    meshes[0].Load("cube.obj");   // Podłoże
    meshes[1].Load("sphere.obj"); // Błyszcząca kula
    meshes[2].Load("monkey.obj"); // Matowa małpka

    // Obiekt 1: Podłoże (Matowe)
    scene.push_back({&meshes[0], glm::vec3(0,-1,0), glm::vec3(0.4, 0.4, 0.4), {0.2f, 0.5f, 0.0f, 1.0f}});
    
    // Obiekt 2: Sfera (Błyszcząca)
    scene.push_back({&meshes[1], glm::vec3(-2, 0, 0), glm::vec3(1, 0, 0), {0.1f, 0.7f, 1.0f, 128.0f}});
    
    // Obiekt 3: Małpka (Pół-mat)
    scene.push_back({&meshes[2], glm::vec3(2, 0, 0), glm::vec3(0, 1, 0), {0.2f, 0.8f, 0.3f, 16.0f}});

    // Inicjalizacja macierzy rzutowania
    matProj = glm::perspective(glm::radians(80.0f), windowWidth/(float)windowHeight, 0.1f, 50.0f);
}

void DisplayScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(idProgram);

    // Wykorzystanie funkcji z utilities.hpp
    glm::mat4 matView = UpdateViewMatrix();
    glm::vec3 lightPos = glm::vec3(2.0f, 4.0f, 2.0f);
    glm::vec3 camPos = ExtractCameraPos(matView);

    glUniformMatrix4fv(glGetUniformLocation(idProgram, "matProj"), 1, GL_FALSE, glm::value_ptr(matProj));
    glUniformMatrix4fv(glGetUniformLocation(idProgram, "matView"), 1, GL_FALSE, glm::value_ptr(matView));
    glUniform3fv(glGetUniformLocation(idProgram, "lightPos"), 1, glm::value_ptr(lightPos));
    glUniform3fv(glGetUniformLocation(idProgram, "viewPos"), 1, glm::value_ptr(camPos));
    glUniform1i(glGetUniformLocation(idProgram, "useLighting"), useLighting);
    glUniform1i(glGetUniformLocation(idProgram, "useBlinnPhong"), useBlinnPhong);

    for (const auto& obj : scene) {
        glm::mat4 matModel = glm::translate(glm::mat4(1.0), obj.position);
        if(obj.mesh == &meshes[0]) matModel = glm::scale(matModel, glm::vec3(10, 0.1, 10));

        glUniformMatrix4fv(glGetUniformLocation(idProgram, "matModel"), 1, GL_FALSE, glm::value_ptr(matModel));
        glUniform3fv(glGetUniformLocation(idProgram, "uColor"), 1, glm::value_ptr(obj.color));
        
        glUniform1f(glGetUniformLocation(idProgram, "material.ambient"), obj.mat.ambient);
        glUniform1f(glGetUniformLocation(idProgram, "material.diffuse"), obj.mat.diffuse);
        glUniform1f(glGetUniformLocation(idProgram, "material.specular"), obj.mat.specular);
        glUniform1f(glGetUniformLocation(idProgram, "material.shininess"), obj.mat.shininess);

        obj.mesh->Draw();
    }
}

// Nadpisujemy key_callback, aby dodać obsługę L i B przy zachowaniu ESC
void my_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    key_callback(window, key, scancode, action, mods); // Wywołaj bazowy z utilities
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_L) { useLighting = !useLighting; printf("Lighting: %d\n", useLighting); }
        if (key == GLFW_KEY_B) { useBlinnPhong = !useBlinnPhong; printf("Blinn-Phong: %d\n", useBlinnPhong); }
    }
}

int main() {
    if (!glfwInit()) return -1;
    glfwSetErrorCallback(error_callback);

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, NULL, NULL);
    if (!window) return -1;

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // Rejestracja callbacków z utilities.hpp
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, my_key_callback);

    Initialize();

    while (!glfwWindowShouldClose(window)) {
        DisplayScene();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    for(auto& m : meshes) m.Release();
    glfwTerminate();
    return 0;
}