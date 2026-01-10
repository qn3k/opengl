#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>

// -------------------------------------------------
// Plik do obslugi plikow graficznych
// -------------------------------------------------
# define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// -------------------------------------------------------
// Zmienne globalne wymagane przez utilities.hpp
// -------------------------------------------------------
int windowWidth = 800;
int windowHeight = 600;
const char *windowTitle = "Scena 3D: OOP i Teksturowanie";

// -------------------------------------------------------
// INCLUDE NAGŁÓWKÓW POMOCNICZYCH
// -------------------------------------------------------
#include "utilities.hpp" 
#include "objloader.hpp" 

// -------------------------------------------------------
// KLASA: CMesh
// Zarządza wczytywaniem OBJ i tworzeniem buforów (VAO/VBO)
// -------------------------------------------------------
class CMesh {
public:
    GLuint idVAO;
    GLuint idVBO_pos;
    GLuint idVBO_norm;
    GLuint idVBO_uv; 
    int vertexCount;

    CMesh() : idVAO(0), idVBO_uv(0), vertexCount(0) {}

    // Metoda wczytująca plik i tworząca bufory
    bool Load(const char* path) {
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec3> normals;

        // Wywołanie funkcji z objloader.hpp
        if (!loadOBJ(path, vertices, uvs, normals)) {
            printf("Nie udalo sie wczytac modelu: %s\n", path);
            return false;
        }

        vertexCount = vertices.size();

        // Generowanie VAO
        glGenVertexArrays(1, &idVAO);
        glBindVertexArray(idVAO);

        // VBO: Pozycje (location = 0)
        glGenBuffers(1, &idVBO_pos);
        glBindBuffer(GL_ARRAY_BUFFER, idVBO_pos);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);

        // VBO: Normalne (location = 2)
        glGenBuffers(1, &idVBO_norm);
        glBindBuffer(GL_ARRAY_BUFFER, idVBO_norm);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(2);
        
        // VBO: Wspolrzedne UV (location = 1)
        if (uvs.size() > 0) {
            glGenBuffers(1, &idVBO_uv);
            glBindBuffer(GL_ARRAY_BUFFER, idVBO_uv);
            glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(1);
        }

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
        glDeleteBuffers(1, &idVBO_uv); 
        glDeleteVertexArrays(1, &idVAO);
    }
};

// -------------------------------------------------------
// FUNKCJA POMOCNICZA: Ładowanie tekstury
// -------------------------------------------------------
GLuint LoadTexture(const char* path) {
    GLuint textureID;
    int tex_width, tex_height, tex_n;
    unsigned char *tex_data;

    stbi_set_flip_vertically_on_load(true);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    tex_data = stbi_load (path, &tex_width , &tex_height , &tex_n, 0);
    if ( tex_data == NULL) {
        printf ("Image can't be loaded: %s\n", path);
        return 0;
    }

    GLenum format = GL_RGB;
    if (tex_n == 4) format = GL_RGBA;
    else if (tex_n == 3) format = GL_RGB;
    else if (tex_n == 1) format = GL_RED;
    
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, tex_width, tex_height, 0, format, GL_UNSIGNED_BYTE, tex_data);
    
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    
    stbi_image_free(tex_data);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    printf("Zaladowano teksture: %s (Format: %d kanalow)\n", path, tex_n);
    return textureID;
}

// -------------------------------------------------------
// STRUKTURA: SceneObject
// -------------------------------------------------------
struct SceneObject {
    CMesh* mesh;             
    glm::vec3 position;      
    glm::vec3 rotation;      
    glm::vec3 scale;         
    glm::vec3 color;         
    GLuint idTexture; //Identyfikator tekstury (0 - brak)       
};

// -------------------------------------------------------
// Zmienne globalne aplikacji
// -------------------------------------------------------
GLuint idProgram;

// Kontenery danych
std::vector<CMesh> meshes;      
std::vector<SceneObject> scene; 
std::vector<GLuint> textures; //Kontener na wczytane tekstury

// Kamera
float cameraDist = 8.0f;
float cameraAngleX = 0.4f;
float cameraAngleY = 0.0f;

// Macierze
glm::mat4 matProj;
glm::mat4 matView;


// -------------------------------------------------------
// Inicjalizacja Sceny
// -------------------------------------------------------
void Initialize()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); 
    glEnable(GL_DEPTH_TEST);
    //bledowanaie
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 1. Szadery
    idProgram = glCreateProgram();
    glAttachShader(idProgram, LoadShader(GL_VERTEX_SHADER, "vertex.glsl"));
    glAttachShader(idProgram, LoadShader(GL_FRAGMENT_SHADER, "fragment.glsl"));
    LinkAndValidateProgram(idProgram);

    // 2. Wczytywanie modeli
    CMesh meshLogo1, meshLogo2, meshLogo3;
    
    //obj
    //v1
    //if(!meshLogo.Load("logo_49blot.obj")) printf("Blad logo\n");
    //v2
    if(!meshLogo1.Load("49blot_1.obj")) printf("Blad logo 1\n");
    if(!meshLogo2.Load("49blot_2.obj")) printf("Blad logo 2\n");
    if(!meshLogo3.Load("49blot_3.obj")) printf("Blad logo 3\n");

    meshes.push_back(meshLogo1); // indeks 0
    meshes.push_back(meshLogo2); // indeks 1
    meshes.push_back(meshLogo3); // indeks 2
    
    // 2.5 Wczytywanie tekstur
    GLuint texGrass = LoadTexture("grass.png"); 
    GLuint texMetal = LoadTexture("metal.png");
    GLuint texBrick = LoadTexture("brick.png");
    GLuint texFlower = LoadTexture("flower32bit.png");
    
    textures.push_back(0); // indeks 0: (domyslna/pusta)
    textures.push_back(texGrass);   // indeks 1
    textures.push_back(texMetal);   // indeks 2
    textures.push_back(texBrick);   // indeks 3
    textures.push_back(texFlower); // indeks 4


    // 3. Tworzenie obiektów na scenie

    // --- OBIEKT 1: Logo (Kolor) ---
    SceneObject logo1;
    logo1.mesh = &meshes[0]; 
    logo1.position = glm::vec3(0.0f, 0.0f, 0.0f);
    logo1.rotation = glm::vec3(0.0f, -1.0f, 3.14f);
    logo1.scale = glm::vec3(-1.0f, 1.0f, 1.0f);
    logo1.color = glm::vec3(0.75f, 0.75f, 0.75f); //jasnoszare
    logo1.idTexture = 0;
    scene.push_back(logo1);

    // --- OBIEKT 2: Logo (Kolor) ---
    SceneObject logo2;
    logo2.mesh = &meshes[1]; 
    logo2.position = glm::vec3(0.0f, 0.0f, -0.5f);
    logo2.rotation = glm::vec3(0.0f, -1.0f, 3.14f);
    logo2.scale = glm::vec3(-1.0f, 1.0f, 1.0f);
    logo2.color = glm::vec3(1.0f, 0.0f, 0.0f); //czerwone
    logo2.idTexture = 0;
    scene.push_back(logo2);

    // --- OBIEKT 3: Logo (Kolor) ---
    SceneObject logo3;
    logo3.mesh = &meshes[2]; 
    logo3.position = glm::vec3(0.0f, 0.0f, -0.5f);
    logo3.rotation = glm::vec3(0.0f, -1.0f, 3.14f);
    logo3.scale = glm::vec3(-1.0f, 1.0f, 1.0f);
    logo3.color = glm::vec3(1.0f, 1.0f, 1.0f); //biale
    logo3.idTexture = 0;
    scene.push_back(logo3);

    // Macierz projekcji
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

    // 1. Ustawienie Kamery
    matView = glm::mat4(1.0);
    matView = glm::translate(matView, glm::vec3(0.0f, 0.0f, -cameraDist)); 
    matView = glm::rotate(matView, cameraAngleX, glm::vec3(1.0f, 0.0f, 0.0f)); 
    matView = glm::rotate(matView, cameraAngleY, glm::vec3(0.0f, 1.0f, 0.0f)); 
    
    glUniformMatrix4fv(glGetUniformLocation(idProgram, "matProj"), 1, GL_FALSE, glm::value_ptr(matProj));
    glUniformMatrix4fv(glGetUniformLocation(idProgram, "matView"), 1, GL_FALSE, glm::value_ptr(matView));

    // 2. Animacje
    float time = glfwGetTime();

    if(scene.size() > 0) {
        scene[0].rotation.y = sin(time) * 0.3f;
        scene[1].rotation.y = sin(time) * 0.3f;
        scene[2].rotation.y = sin(time) * 0.3f;
    }


    // 3. Pętla rysująca wszystkie obiekty
    GLint loc_uColor = glGetUniformLocation(idProgram, "uColor");
    GLint loc_matModel = glGetUniformLocation(idProgram, "matModel");
    GLint loc_sampler = glGetUniformLocation(idProgram, "uTextureSampler"); 
    GLint loc_bUseTexture = glGetUniformLocation(idProgram, "bUseTexture");
    
    for (const auto& obj : scene)
    {
        if(!obj.mesh) continue;

        // Obliczanie macierzy modelu
        glm::mat4 matModel = glm::mat4(1.0);
        matModel = glm::translate(matModel, obj.position);
        matModel = glm::rotate(matModel, obj.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        matModel = glm::rotate(matModel, obj.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        matModel = glm::rotate(matModel, obj.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        matModel = glm::scale(matModel, obj.scale);

        // Przekazanie uniformów
        glUniformMatrix4fv(loc_matModel, 1, GL_FALSE, glm::value_ptr(matModel));
        glUniform3fv(loc_uColor, 1, glm::value_ptr(obj.color));
        
        // Obsługa tekstury
        if (obj.idTexture != 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, obj.idTexture);
            glUniform1i(loc_sampler, 0); 
            glUniform1i(loc_bUseTexture, 1); // Użyj tekstury
        } else {
            glUniform1i(loc_bUseTexture, 0); // Użyj koloru
        }

        // Rysowanie
        obj.mesh->Draw();
        
        // Opcjonalne: wyłącz wiązanie tekstury
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glUseProgram(0);
}

// -------------------------------------------------------
// Implementacja callbacków zadeklarowanych w utilities.hpp
// -------------------------------------------------------

// Zmiana rozmiaru okna
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
    if (height > 0)
        matProj = glm::perspective(glm::radians(60.0f), width/(float)height, 0.1f, 100.0f);
}

// Klawiatura (WSAD i Zoom)
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        switch(key)
        {
            case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
            
            // Obrót kamery
            case GLFW_KEY_W: cameraAngleX += 0.05f; break;
            case GLFW_KEY_S: cameraAngleX -= 0.05f; break;
            case GLFW_KEY_A: cameraAngleY -= 0.05f; break;
            case GLFW_KEY_D: cameraAngleY += 0.05f; break;

            // Zoom (+/-)
            case GLFW_KEY_EQUAL:      
            case GLFW_KEY_KP_ADD:     
                cameraDist -= 0.5f; 
                if (cameraDist < 2.0f) cameraDist = 2.0f; 
                break;
            case GLFW_KEY_MINUS:      
            case GLFW_KEY_KP_SUBTRACT: 
                cameraDist += 0.5f; 
                break;
        }
    }
}

// Scroll (opcjonalnie też do zoomu)
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    cameraDist -= yoffset;
    if (cameraDist < 2.0f) cameraDist = 2.0f;
}

// Ruch myszy (opcjonalne)
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    // obsługa myszy
}


// -------------------------------------------------------
// MAIN
// -------------------------------------------------------
int main(int argc, char *argv[])
{
    // Inicjalizacja okna i OpenGL (funkcja z utilities.hpp)
    GLFWwindow* window = nullptr;
    Initialize_GLFW(window);

    // Inicjalizacja naszych danych
    Initialize();

    // Główna pętla
    while (!glfwWindowShouldClose(window))
    {
        // Aktualizacja i renderowanie
        DisplayScene();
        
        // Obsługa zdarzeń i wymiana buforów
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    // Sprzątanie
    glDeleteProgram(idProgram);
    for(auto& m : meshes) {
        m.Release();
    }
    // CZYSZCZENIE: tekstur
    for(auto& t : textures) {
        glDeleteTextures(1, &t);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}