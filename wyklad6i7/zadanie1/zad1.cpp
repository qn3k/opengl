#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>

// -------------------------------------------------
// NOWY: Plik do obslugi plikow graficznych
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
    CMesh meshGround, meshCube, meshSphere, meshMonkey;
    
    //obj
    if(!meshGround.Load("cube.obj")) printf("Blad ground\n"); 
    if(!meshCube.Load("cube.obj")) printf("Blad cube\n");
    if(!meshSphere.Load("sphere.obj")) printf("Blad sphere\n");
    if(!meshMonkey.Load("monkey.obj")) printf("Blad monkey\n");

    //static
    CMesh meshFlower = CMesh::CreateFlowerMesh();

    meshes.push_back(meshGround); // indeks 0
    meshes.push_back(meshCube);   // indeks 1
    meshes.push_back(meshSphere); // indeks 2
    meshes.push_back(meshMonkey); // indeks 3
    meshes.push_back(meshFlower); // indeks 4
    
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

    // --- OBIEKT 1: Podłoże (Teksturowane cegłą) ---
    SceneObject ground;
    ground.mesh = &meshes[0]; 
    ground.position = glm::vec3(0.0f, -1.0f, 0.0f);
    ground.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    ground.scale = glm::vec3(8.0f, 0.1f, 8.0f); 
    ground.color = glm::vec3(0.2f, 0.5f, 0.2f); 
    ground.idTexture = textures[1]; // Tekstura trawy
    scene.push_back(ground);

    // --- OBIEKT 2: Sześcian (cegly) ---
    SceneObject box;
    box.mesh = &meshes[1];
    box.position = glm::vec3(-2.0f, 0.0f, -2.0f);
    box.rotation = glm::vec3(0.0f);
    box.scale = glm::vec3(1.0f);
    box.color = glm::vec3(0.2f, 0.3f, 0.8f); 
    box.idTexture = textures[3]; // cegly
    scene.push_back(box);

    // --- OBIEKT 3: Sfera (Teksturowana metalem) ---
    SceneObject ball;
    ball.mesh = &meshes[2];
    ball.position = glm::vec3(2.0f, 1.0f, -1.0f);
    ball.rotation = glm::vec3(0.0f);
    ball.scale = glm::vec3(1.2f);
    ball.color = glm::vec3(0.9f, 0.2f, 0.2f);
    ball.idTexture = textures[2]; // Tekstura metalu
    scene.push_back(ball);

    // --- OBIEKT 4: Małpka (Kolor) ---
    SceneObject suzanne;
    suzanne.mesh = &meshes[3]; 
    suzanne.position = glm::vec3(0.0f, 0.5f, 1.0f);
    suzanne.rotation = glm::vec3(0.0f, 3.14f, 0.0f); 
    suzanne.scale = glm::vec3(1.0f);
    suzanne.color = glm::vec3(1.0f, 0.8f, 0.1f); // Złoty kolor
    suzanne.idTexture = 0;
    scene.push_back(suzanne);

    // --- OBIEKT 5: MONOLIT (Kolor) ---
    if(meshes.size() > 1) {
        SceneObject monolith;
        monolith.mesh = &meshes[1];
        
        monolith.position = glm::vec3(3.5f, 0.5f, 3.5f); 
        monolith.rotation = glm::vec3(0.0f, 0.78f, 0.0f); 
        monolith.scale = glm::vec3(0.5f, 1.5f, 0.5f); 
        monolith.color = glm::vec3(0.2f, 0.3f, 0.8f);  
        monolith.idTexture = 0;
        
        scene.push_back(monolith);
    }

    // --- OBIEKT 6: KWIAT (tekstura) ---
    SceneObject flower1;
    flower1.mesh = &meshes[4]; // Wskaźnik na nowo utworzoną siatkę kwiatu
    flower1.position = glm::vec3(4.0f, -0.5f, -3.0f); 
    flower1.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    flower1.scale= glm::vec3(0.5f, 0.5f, 0.5f); //  mniejszy
    flower1.color = glm::vec3(1.0f); // Kolor biały 
    flower1.idTexture = textures[4]; // Tekstura kwiatu
    scene.push_back(flower1);
    
    SceneObject flower2 = flower1; // Kopiujemy
    flower2.position = glm::vec3(-3.0f, -0.5f, 3.0f);
    flower2.scale = glm::vec3(0.8f, 0.8f, 0.8f);
    flower2.rotation = glm::vec3(0.0f, 1.57f, 0.0f); // Obrócony o 90 stopni 
    scene.push_back(flower2);

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

    if(scene.size() > 1) {
        scene[1].rotation.y = time * 1.5f; 
        scene[1].rotation.x = time * 0.5f;
    }
    if(scene.size() > 2) {
        scene[2].position.y = 1.0f + sin(time * 3.0f) * 0.5f;
    }
    if(scene.size() > 3) {
        scene[3].rotation.z = sin(time) * 0.3f;
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