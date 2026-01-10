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
// Zmienne globalne wymagane przez utilities.hpp
// Muszą być zdefiniowane PRZED include "utilities.hpp"
// -------------------------------------------------------
int windowWidth = 800;
int windowHeight = 600;
const char *windowTitle = "Scena 3D: OOP i Instancing";

// -------------------------------------------------------
// INCLUDE NAGŁÓWKÓW POMOCNICZYCH
// -------------------------------------------------------
#include "utilities.hpp" // Twój plik
#include "objloader.hpp" // Zakładam, że ten plik istnieje w projekcie

// -------------------------------------------------------
// KLASA: CMesh
// Cel: Zarządza wczytywaniem OBJ i tworzeniem buforów (VAO/VBO)
// Eliminuje zmienne typu idVAO1, idVAO2...
// -------------------------------------------------------
class CMesh {
public:
    GLuint idVAO;
    GLuint idVBO_pos;
    GLuint idVBO_norm; // Używamy normalnych do oświetlenia/gradientu
    int vertexCount;

    CMesh() : idVAO(0), vertexCount(0) {}

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

        // VBO: Normalne (location = 2) - ważne dla oświetlenia
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
// Cel: Reprezentuje konkretny obiekt na scenie.
// Pozwala użyć jednego modelu (CMesh) wiele razy w różnych miejscach.
// -------------------------------------------------------
struct SceneObject {
    CMesh* mesh;            // Wskaźnik na geometrię (nie kopiujemy danych!)
    glm::vec3 position;     // Gdzie jest
    glm::vec3 rotation;     // Jak obrócony (Euler angles: x, y, z)
    glm::vec3 scale;        // Jak duży
    glm::vec3 color;        // Jakiego koloru
};

// -------------------------------------------------------
// Zmienne globalne aplikacji
// -------------------------------------------------------
GLuint idProgram;

// Kamera
float cameraDist = 8.0f;
float cameraAngleX = 0.4f; // Lekkie pochylenie na start
float cameraAngleY = 0.0f;

// Kontenery danych
std::vector<CMesh> meshes;      // Przechowuje wczytane modele (dane geometryczne)
std::vector<SceneObject> scene; // Przechowuje listę obiektów do wyrenderowania

// Macierze
glm::mat4 matProj;
glm::mat4 matView;


// -------------------------------------------------------
// Inicjalizacja Sceny
// -------------------------------------------------------
void Initialize()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Ciemne tło
    glEnable(GL_DEPTH_TEST);

    // 1. Szadery
    idProgram = glCreateProgram();
    glAttachShader(idProgram, LoadShader(GL_VERTEX_SHADER, "vertex.glsl"));
    glAttachShader(idProgram, LoadShader(GL_FRAGMENT_SHADER, "fragment.glsl"));
    LinkAndValidateProgram(idProgram);

    // 2. Wczytywanie modeli (Tylko raz dla każdego typu!)
    CMesh meshGround, meshCube, meshSphere, meshMonkey;
    
    // Używam prostych nazw plików 
    if(!meshGround.Load("cube.obj")) printf("Blad ground\n");   // Użyjemy sześcianu jako płaskiej podłogi
    if(!meshCube.Load("cube.obj")) printf("Blad cube\n");
    if(!meshSphere.Load("sphere.obj")) printf("Blad sphere\n");
    if(!meshMonkey.Load("monkey.obj")) printf("Blad monkey\n");

    meshes.push_back(meshGround); // indeks 0
    meshes.push_back(meshCube);   // indeks 1
    meshes.push_back(meshSphere); // indeks 2
    meshes.push_back(meshMonkey); // indeks 3

    // 3. Tworzenie obiektów na scenie

    // --- OBIEKT 1: Podłoże (Zielone) ---
    SceneObject ground;
    ground.mesh = &meshes[0]; // Wskaźnik na pierwszy mesh
    ground.position = glm::vec3(0.0f, -1.0f, 0.0f);
    ground.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    ground.scale    = glm::vec3(8.0f, 0.1f, 8.0f); // Spłaszczamy sześcian
    ground.color    = glm::vec3(0.2f, 0.5f, 0.2f); // Ciemna zieleń
    scene.push_back(ground);

    // --- OBIEKT 2: Sześcian (Niebieski) ---
    SceneObject box;
    box.mesh = &meshes[1];
    box.position = glm::vec3(-2.0f, 0.0f, -2.0f);
    box.rotation = glm::vec3(0.0f);
    box.scale    = glm::vec3(1.0f);
    box.color    = glm::vec3(0.2f, 0.3f, 0.8f); 
    scene.push_back(box);

    // --- OBIEKT 3: Sfera (Czerwona, lewitująca) ---
    SceneObject ball;
    ball.mesh = &meshes[2];
    ball.position = glm::vec3(2.0f, 1.0f, -1.0f);
    ball.rotation = glm::vec3(0.0f);
    ball.scale    = glm::vec3(1.2f);
    ball.color    = glm::vec3(0.9f, 0.2f, 0.2f);
    scene.push_back(ball);

    // --- OBIEKT 4: Małpka (Złota, na środku) ---
    SceneObject suzanne;
    suzanne.mesh = &meshes[3]; 
    suzanne.position = glm::vec3(0.0f, 0.5f, 1.0f);
    suzanne.rotation = glm::vec3(0.0f, 3.14f, 0.0f); // Odwrócona przodem
    suzanne.scale    = glm::vec3(1.0f);
    suzanne.color    = glm::vec3(1.0f, 0.8f, 0.1f);
    scene.push_back(suzanne);

    // --- OBIEKT 5: NIERUCHOMY MONOLIT (Przyklejony do podłoża) ---
    if(meshes.size() > 1) {
        SceneObject monolith;
        monolith.mesh = &meshes[1]; // Używamy cube.obj
        
        monolith.position = glm::vec3(3.5f, 0.5f, 3.5f); 
        monolith.rotation = glm::vec3(0.0f, 0.78f, 0.0f); // Lekko obrócony (45 stopni)
        monolith.scale    = glm::vec3(0.5f, 1.5f, 0.5f);  // Wąski i wysoki
        monolith.color    = glm::vec3(0.4f, 0.4f, 0.45f); // Szary, kamienny kolor
        
        scene.push_back(monolith);
    }

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

    // 1. Ustawienie Kamery (WSAD + Zoom)
    matView = glm::mat4(1.0);
    matView = glm::translate(matView, glm::vec3(0.0f, 0.0f, -cameraDist)); // Zoom (oddalenie)
    matView = glm::rotate(matView, cameraAngleX, glm::vec3(1.0f, 0.0f, 0.0f)); // Góra/Dół
    matView = glm::rotate(matView, cameraAngleY, glm::vec3(0.0f, 1.0f, 0.0f)); // Lewo/Prawo
    
    glUniformMatrix4fv(glGetUniformLocation(idProgram, "matProj"), 1, GL_FALSE, glm::value_ptr(matProj));
    glUniformMatrix4fv(glGetUniformLocation(idProgram, "matView"), 1, GL_FALSE, glm::value_ptr(matView));

    // 2. Animacje (Aktualizacja stanu obiektów)
    float time = glfwGetTime();

    // Animacja A: Obrót sześcianu (obiekt nr 1 w liście scene, bo 0 to podłoga)
    if(scene.size() > 1) {
        scene[1].rotation.y = time * 1.5f; 
        scene[1].rotation.x = time * 0.5f;
    }

    // Animacja B: Skakanie sfery (obiekt nr 2)
    if(scene.size() > 2) {
        scene[2].position.y = 1.0f + sin(time * 3.0f) * 0.5f;
    }

    // Animacja C: Małpka (obiekt nr 3) buja się na boki
    if(scene.size() > 3) {
        scene[3].rotation.z = sin(time) * 0.3f;
    }


    // 3. Pętla rysująca wszystkie obiekty
    for (const auto& obj : scene)
    {
        // Jeśli obiekt nie ma przypisanej siatki, pomiń
        if(!obj.mesh) continue;

        // Obliczanie macierzy modelu dla konkretnego obiektu
        glm::mat4 matModel = glm::mat4(1.0);
        matModel = glm::translate(matModel, obj.position);
        matModel = glm::rotate(matModel, obj.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        matModel = glm::rotate(matModel, obj.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        matModel = glm::rotate(matModel, obj.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        matModel = glm::scale(matModel, obj.scale);

        // Przekazanie uniformów do shadera
        glUniformMatrix4fv(glGetUniformLocation(idProgram, "matModel"), 1, GL_FALSE, glm::value_ptr(matModel));
        glUniform3fv(glGetUniformLocation(idProgram, "uColor"), 1, glm::value_ptr(obj.color));

        // Rysowanie
        obj.mesh->Draw();
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

// Ruch myszy (opcjonalne, zostawiam puste jeśli chcesz sterować tylko WSAD)
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

    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}