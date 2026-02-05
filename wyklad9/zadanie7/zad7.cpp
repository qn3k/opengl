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
const char* windowTitle = "Projekt beta v0.2";
glm::mat4 matProj;
glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f); // Domyślnie białe


#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "utilities.hpp" 
#include "objloader.hpp" 
#include "SceneManager.hpp"
#include "skybox.h"
#include "ground.hpp"
#include "player.hpp"
#include "shadow-dir.hpp"
#include "PlayerLogic.hpp"

CGround myGround;
CPlayer myPlayer;

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
bool showMinimap = true; // Flaga widoczności minimapy
bool useShadows = true; // Flaga kontrolna
bool canMove = true;
bool playerArrow = true;
float playerRadius = 0.5f;
extern std::vector<glm::mat4> treeMatrices;
float normalStrength = 1.0f;

// --- STRUKTURA ŚWIATŁA ---
struct PointLight {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
};

PointLight lights[4];
int activeLightsCount = 1; // Startujemy z jednym światłem
//glm::mat4 lightProj, lightView; //macierze swiatla do shaddow mappingu


// Zmienne
GLuint idProgram;
std::vector<CMesh> meshes;
std::vector<SceneObject> scene;
std::vector<GLuint> textures;
bool keys[1024];
int playerIdx = -1;

// Stałe dla czytelności (jak w przykładzie który Ci się podobał)
enum { SCREEN = 0 }; 
GLuint idVAO[1]; 
GLint loc_matPVM, loc_tex;

// Parametry FBO (Minimapy)
GLuint fbo, idTextureBuffer, rbo;
int bufferWidth = 1600;
int bufferHeight = 900;

GLfloat vertices_pos[] = {
    -1.0f, -1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
     1.0f,  1.0f, 0.0f,
     1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f,
};

void setupFBO() {
    // 1. Tworzymy Framebuffer
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // 2. Tworzymy teksturę, do której będziemy renderować
    glGenTextures(1, &idTextureBuffer);
    glBindTexture(GL_TEXTURE_2D, idTextureBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Podpinamy teksturę do FBO jako załącznik koloru
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, idTextureBuffer, 0);

    // 3. Tworzymy Renderbuffer dla Depth i Stencil (żeby działał Depth Test)
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
    
    // Podpinamy RBO pod test głębi
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    // Sprawdzamy, czy wszystko OK
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Wracamy do domyślnego bufora
}

void DrawWorld(glm::mat4 projection, glm::mat4 view, glm::vec3 camPos) {
   
    glm::mat4 viewStatic = glm::mat4(glm::mat3(view)); // Usuwamy translację

    //glUseProgram(idProgram);

    glActiveTexture(GL_TEXTURE1);
    if (currentSkybox == 0) glBindTexture(GL_TEXTURE_CUBE_MAP, skybox1->getTextureID());
    else glBindTexture(GL_TEXTURE_CUBE_MAP, skybox2->getTextureID());
    glUniform1i(glGetUniformLocation(idProgram, "tex_skybox"), 1);

    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);

    if (currentSkybox == 0 && skybox1) {
        skybox1->draw(projection, viewStatic, 100.0f); // Zwiększyłem dystans na 100
    } 
    else if (currentSkybox == 1 && skybox2) {
        skybox2->draw(projection, viewStatic, 100.0f);
    }

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

    glUseProgram(idProgram);
    
    // Pobieramy lokalizacje zmiennych, których wcześniej brakowało
    GLint loc_bUseTexture = glGetUniformLocation(idProgram, "bUseTexture");
    GLint locTiling = glGetUniformLocation(idProgram, "uTiling");

    // Przesyłanie macierzy - używamy nazw z argumentów funkcji (projection, view, camPos)
    glUniformMatrix4fv(glGetUniformLocation(idProgram, "matProj"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(idProgram, "matView"), 1, GL_FALSE, glm::value_ptr(view));
    glUniform3fv(glGetUniformLocation(idProgram, "viewPos"), 1, glm::value_ptr(camPos));

    // --- OŚWIETLENIE ---
    float time = (float)glfwGetTime();
    for (int i = 0; i < 4; i++) {
        if (animateLight) {
            float angle = time * lightOrbitSpeed + (i * 1.57f);
            float radius = 4.0f + i;
            lights[i].position = glm::vec3(sin(angle) * radius, 2.5f, cos(angle) * radius);
        }
    }

    glUniform1i(glGetUniformLocation(idProgram, "useLighting"), useLighting);
    glUniform1i(glGetUniformLocation(idProgram, "useBlinnPhong"), useBlinnPhong);
    glUniform3fv(glGetUniformLocation(idProgram, "dirLightDirection"), 1, glm::value_ptr(dirLightDirection));
    glUniform1i(glGetUniformLocation(idProgram, "isPointLight"), isPointLight);
    glUniform1i(glGetUniformLocation(idProgram, "activeLightsCount"), activeLightsCount);

    for (int i = 0; i < 4; i++) {
        std::string base = "lights[" + std::to_string(i) + "]";
        if(i == 0) lights[i].color = lightColor; 
        glUniform3fv(glGetUniformLocation(idProgram, (base + ".position").c_str()), 1, glm::value_ptr(lights[i].position));
        glUniform3fv(glGetUniformLocation(idProgram, (base + ".color").c_str()), 1, glm::value_ptr(lights[i].color));
        glUniform1f(glGetUniformLocation(idProgram, (base + ".intensity").c_str()), lights[i].intensity);
    }

    // --- INSTANCJONOWANIE ---
    glUniform1i(glGetUniformLocation(idProgram, "bIsInstanced"), 1);
    //kwiaty
    glUniform1i(loc_bUseTexture, 1); 
    glUniform1f(locTiling, 1.0f); 
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[4]);
    glUniform1i(glGetUniformLocation(idProgram, "uTextureSampler"), 0);
    meshes[4].Draw(); 
    //drzewa
    glUniform1i(loc_bUseTexture, 0);
    glUniform1f(glGetUniformLocation(idProgram, "material.ambient"), 0.3f);
    glUniform1f(glGetUniformLocation(idProgram, "material.specular"), 0.1f);
    glUniform1f(glGetUniformLocation(idProgram, "material.shininess"), 32.0f);
    glUniform3f(glGetUniformLocation(idProgram, "uColor"), 1.0f, 1.0f, 1.0f);
    meshes[11].Draw(); 


    //powrot do normalnych obiektow
    glUniform1i(glGetUniformLocation(idProgram, "bIsInstanced"), 0);
    glUniform1i(loc_bUseTexture, 1);

    // shaddow mapping
    glUniformMatrix4fv(glGetUniformLocation(idProgram, "lightProj"), 1, GL_FALSE, glm::value_ptr(lightProj));
    glUniformMatrix4fv(glGetUniformLocation(idProgram, "lightView"), 1, GL_FALSE, glm::value_ptr(lightView));

    // Przekazujemy teksturę cienia na SLOT 2 (Slot 0 - tekstura obiektu, Slot 1 - Skybox)
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, DepthMap_idTexture);
    glUniform1i(glGetUniformLocation(idProgram, "tex_shadowMap"), 2);

    GLint loc_uNormalMap = glGetUniformLocation(idProgram, "uNormalMap");
    GLint loc_uUseNormalMap = glGetUniformLocation(idProgram, "uUseNormalMap");

    // --- OBIEKTY SCENY ---
    for (const auto& obj : scene) {
        if (obj.isCollected) continue;
        glm::mat4 matModel = glm::mat4(1.0);
        matModel = glm::translate(matModel, obj.position);
        matModel = glm::rotate(matModel, obj.rotation.y, glm::vec3(0, 1, 0));  
        matModel = glm::rotate(matModel, obj.rotation.x, glm::vec3(1, 0, 0)); 
        matModel = glm::rotate(matModel, obj.rotation.z, glm::vec3(0, 0, 1)); 
        matModel = glm::scale(matModel, obj.scale);
        
        glUniformMatrix4fv(glGetUniformLocation(idProgram, "matModel"), 1, GL_FALSE, glm::value_ptr(matModel));

        // Materiał
        glUniform1f(glGetUniformLocation(idProgram, "material.ambient"), obj.mat.ambient);
        glUniform1f(glGetUniformLocation(idProgram, "material.diffuse"), obj.mat.diffuse);
        glUniform1f(glGetUniformLocation(idProgram, "material.specular"), obj.mat.specular);
        glUniform1f(glGetUniformLocation(idProgram, "material.shininess"), obj.mat.shininess);

        // Tekstura
        if (obj.idTexture > 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, obj.idTexture);
            glUniform1i(glGetUniformLocation(idProgram, "uTextureSampler"), 0);
            glUniform1i(loc_bUseTexture, 1);
        } else {
            glUniform1i(loc_bUseTexture, 0);
        }

        // idNormalMap
        if (obj.idNormalMap > 0) { 
            glActiveTexture(GL_TEXTURE3); // Używamy slotu 3 (0-Albedo, 1-Skybox, 2-Shadow)
            glBindTexture(GL_TEXTURE_2D, obj.idNormalMap);
            glUniform1i(loc_uNormalMap, 3);
            glUniform1i(loc_uUseNormalMap, 1);
        } else {
            glUniform1i(loc_uUseNormalMap, 0);
        }
        glUniform3fv(glGetUniformLocation(idProgram, "uColor"), 1, glm::value_ptr(obj.color));

        // Tiling podłoża
        if (obj.mesh == &meshes[0]) glUniform1f(locTiling, 8.0f); 
        else if (obj.mesh == &meshes[12]) glUniform1f(locTiling, 2.0f);
        else glUniform1f(locTiling, 1.0f);

        // Env mapping (Koliber)
        if (obj.mesh == &meshes[5]) {
            glUniform1i(glGetUniformLocation(idProgram, "uUseEnvMap"), 1);
            glUniform1f(glGetUniformLocation(idProgram, "reflectionFactor"), 0.8f);
        } else {
            glUniform1i(glGetUniformLocation(idProgram, "uUseEnvMap"), 0);
        }

        // Gdzieś w pętli rysowania, przed narysowaniem prostopadłościanu:
        int strengthLoc = glGetUniformLocation(idProgram, "uNormalStrength");
        glUniform1f(strengthLoc, normalStrength); 

        obj.mesh->Draw();
    }

    // --- ŻARÓWKI ---
    if (showLightSource && isPointLight) {
        glUniform1i(glGetUniformLocation(idProgram, "bIsLightSource"), true);
        for (int i = 0; i < activeLightsCount; i++) {
            glm::mat4 matLight = glm::translate(glm::mat4(1.0), lights[i].position);
            matLight = glm::scale(matLight, glm::vec3(0.15f));
            glUniform3fv(glGetUniformLocation(idProgram, "lightColor"), 1, glm::value_ptr(lights[i].color));
            glUniformMatrix4fv(glGetUniformLocation(idProgram, "matModel"), 1, GL_FALSE, glm::value_ptr(matLight));
            meshes[2].Draw();
        }
        glUniform1i(glGetUniformLocation(idProgram, "bIsLightSource"), false);
    }

    //strzalka
}

void DrawWorldForShadows() {
    glUseProgram(DepthMap_idProgram);

    // Przesyłamy wspólne macierze światła (już ustawione w DisplayScene, ale dla pewności)
    glUniformMatrix4fv(glGetUniformLocation(DepthMap_idProgram, "matProj"), 1, GL_FALSE, glm::value_ptr(lightProj));
    glUniformMatrix4fv(glGetUniformLocation(DepthMap_idProgram, "matView"), 1, GL_FALSE, glm::value_ptr(lightView));

    //kwiaty i drzewa
    GLint locInstanced = glGetUniformLocation(DepthMap_idProgram, "bIsInstanced");
    glUniform1i(locInstanced, 1);
    meshes[4].Draw(); 
    meshes[11].Draw(); 
    glUniform1i(locInstanced, 0); // wylaczamy dla zwykłych obiektow

    // 2. Rysujemy obiekty sceny
    for (const auto& obj : scene) {
        // Żarówki nie rzucają cienia
        if (obj.mesh == &meshes[2]) continue; 

        glm::mat4 matModel = glm::mat4(1.0);
        matModel = glm::translate(matModel, obj.position);
        matModel = glm::rotate(matModel, obj.rotation.y, glm::vec3(0, 1, 0));  
        matModel = glm::rotate(matModel, obj.rotation.x, glm::vec3(1, 0, 0)); 
        matModel = glm::rotate(matModel, obj.rotation.z, glm::vec3(0, 0, 1)); 
        matModel = glm::scale(matModel, obj.scale);
        
        glUniformMatrix4fv(glGetUniformLocation(DepthMap_idProgram, "matModel"), 1, GL_FALSE, glm::value_ptr(matModel));

        obj.mesh->Draw();
    }   
}

void RenderScene_to_Texture() {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo); 
    glViewport(0, 0, 800, 600); 
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float zoom = 25.0f; 
    glm::mat4 miniProj = glm::ortho(-zoom, zoom, -zoom, zoom, 0.1f, 100.0f);
    glm::mat4 miniView = glm::lookAt(
        myPlayer.position + glm::vec3(0, 50, 0), 
        myPlayer.position, 
        glm::vec3(0, 0, -1) 
    );

    // 1. Rysujemy świat na minimapie
    DrawWorld(miniProj, miniView, myPlayer.position + glm::vec3(0, 50, 0));

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderScene_on_Screen(glm::mat4 projection, glm::mat4 view, glm::vec3 camPos) {
    // --- KROK 1: RYSOWANIE ŚWIATA (CAŁY EKRAN) ---
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, windowWidth, windowHeight);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Rysujemy świat normalnie
    DrawWorld(projection, view, camPos);

    // --- KROK 2: RYSOWANIE MINIMAPY (TYLKO KWADRAT) ---
    if (showMinimap) {
        glDisable(GL_DEPTH_TEST); 
        glUseProgram(idProgram);

        // Macierz modelu dla minimapy (pozycja i skala w rogu)
        glm::mat4 matModelMinimap = glm::mat4(1.0f);
        matModelMinimap = glm::translate(matModelMinimap, glm::vec3(0.7f, 0.7f, 0.0f)); 
        matModelMinimap = glm::scale(matModelMinimap, glm::vec3(0.25f)); 

        // WAŻNE: Musimy zneutralizować macierze Proj i View dla minimapy,
        // bo minimapa jest już zdefiniowana w NDC (-1 do 1)
        glm::mat4 identity = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(idProgram, "matProj"), 1, GL_FALSE, glm::value_ptr(identity));
        glUniformMatrix4fv(glGetUniformLocation(idProgram, "matView"), 1, GL_FALSE, glm::value_ptr(identity));
        glUniformMatrix4fv(glGetUniformLocation(idProgram, "matModel"), 1, GL_FALSE, glm::value_ptr(matModelMinimap));

        // Wyłączamy oświetlenie dla minimapy (żeby nie była czarna/dziwna)
        glUniform1i(glGetUniformLocation(idProgram, "useLighting"), 0);
        glUniform1f(glGetUniformLocation(idProgram, "uTiling"), 1.0f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, idTextureBuffer); // Tekstura z FBO
        glUniform1i(glGetUniformLocation(idProgram, "uTextureSampler"), 0);
        glUniform1i(glGetUniformLocation(idProgram, "bUseTexture"), 1);

        glBindVertexArray(idVAO[SCREEN]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        glEnable(GL_DEPTH_TEST);
    }
}

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
    for(int i=0; i<scene.size(); i++) {
        if(scene[i].mesh == &meshes[7]) { 
            playerIdx = i;
            break;
        }
    }
    myGround.Init(); 
    myPlayer.Init(&myGround);
    myPlayer.position.y = myGround.getY(glm::vec2(myPlayer.position.x, myPlayer.position.z));

    // Konfiguracja Skyboxa
    std::vector<std::string> faces1 = {"skybox1/posx.jpg","skybox1/negx.jpg","skybox1/posy.jpg","skybox1/negy.jpg","skybox1/posz.jpg","skybox1/negz.jpg"};
    std::vector<std::string> faces2 = {"skybox2/posx.jpg","skybox2/negx.jpg","skybox2/posy.jpg","skybox2/negy.jpg","skybox2/posz.jpg","skybox2/negz.jpg"};

    skybox1 = new Skybox(faces1, "shaders/skybox-vertex.glsl", "shaders/skybox-fragment.glsl");
    skybox2 = new Skybox(faces2, "shaders/skybox-vertex.glsl", "shaders/skybox-fragment.glsl");
    
    matProj = glm::perspective(glm::radians(80.0f), windowWidth/(float)windowHeight, 0.1f, 500.0f);

    // Inicjalizacja kwadratu dla minimapy (SCREEN)
    glGenVertexArrays(1, &idVAO[SCREEN]);
    glBindVertexArray(idVAO[SCREEN]);
    GLuint vbo_pos;
    glGenBuffers(1, &vbo_pos);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_pos), vertices_pos, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    // Dodajemy współrzędne UV dla kwadratu (żeby tekstura z FBO się wyświetliła)
    GLfloat screen_uv[] = { 0,0, 1,0, 1,1, 1,1, 0,1, 0,0 };
    GLuint vbo_uv;
    glGenBuffers(1, &vbo_uv);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_uv);
    glBufferData(GL_ARRAY_BUFFER, sizeof(screen_uv), screen_uv, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

//swiatla kolorowe
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

    handleInput(keys, myPlayer, scene, meshes, pIdx, playerIdx, playerRadius);
    playerAnimation(keys, myPlayer, scene, pIdx);

    //obsluga kamery wokol srodka
    //glm::mat4 matView = UpdateViewMatrix(); 
    //glm::vec3 cameraPos = ExtractCameraPos(matView);  

    // obsluga kamery tpp
    
    float pitch = glm::radians(30.0f); // Kąt nachylenia w dół 
    float camDistZ =4.0f;             // Całkowity dystans od gracza

    // Obliczamy składowe na podstawie kąta nachylenia
    float horizontalDist = camDistZ * cos(pitch);
    float verticalDist = camDistZ * sin(pitch);

    // Obliczamy pozycję kamery (za plecami gracza)
    glm::vec3 cameraPos;

    float angleOffset = 1.57f; 
    cameraPos.x = myPlayer.position.x - horizontalDist * sin(myPlayer.rotationY + angleOffset);
    cameraPos.z = myPlayer.position.z - horizontalDist * cos(myPlayer.rotationY + angleOffset);
    cameraPos.y = myPlayer.position.y + verticalDist;

    // Patrzymy na punkt nieco nad stopami gracza (np. na klatkę piersiową)
    glm::vec3 lookAtPoint = myPlayer.position + glm::vec3(0.0f, 1.0f, 0.0f);

    glm::mat4 matView = glm::lookAt(cameraPos, lookAtPoint, glm::vec3(0.0f, 1.0f, 0.0f));

    //glUniformMatrix4fv(glGetUniformLocation(idProgram, "matView"), 1, GL_FALSE, glm::value_ptr(matView));

    //glm::vec3 cameraOffset = glm::vec3(-5.0f * sin(myPlayer.rotationY), 3.0f, -5.0f * cos(myPlayer.rotationY));
    //glm::vec3 cameraPos = myPlayer.position + cameraOffset;
    //glm::mat4 matView = glm::lookAt(cameraPos, myPlayer.position + glm::vec3(0, 1.5f, 0), glm::vec3(0, 1, 0));

    // 1. Generowanie Mapy Cieni (Tylko głębia)
    if (useShadows) {
        glViewport(0, 0, DepthMap_Width, DepthMap_Height);
        glBindFramebuffer(GL_FRAMEBUFFER, DepthMap_idFrameBuffer);
        glClear(GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(DepthMap_idProgram);
        
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        // Przesyłamy macierze światła
        glUniformMatrix4fv(glGetUniformLocation(DepthMap_idProgram, "matProj"), 1, GL_FALSE, glm::value_ptr(lightProj));
        glUniformMatrix4fv(glGetUniformLocation(DepthMap_idProgram, "matView"), 1, GL_FALSE, glm::value_ptr(lightView));

        // Rysujemy świat (musisz tu wywołać DrawWorld lub pętlę rysującą obiekty)
        DrawWorldForShadows(); 

        glCullFace(GL_BACK); 
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // 2. Render pozaekranowy
    if (showMinimap) {
        RenderScene_to_Texture();
    }

    // 3. Render właściwy
    RenderScene_on_Screen(matProj, matView, cameraPos);

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
            snprintf(buf, sizeof(buf), "Light %d", i + 1);
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

    ImGui::Begin("Ustawienia Materialu");
    ImGui::SliderFloat("Sila Normal Mapy", &normalStrength, 0.0f, 5.0f); // Zakres od 0 (płaskie) do 5 (bardzo wypukłe)
    ImGui::End();

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// Nadpisujemy key_callback
void my_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    key_callback(window, key, scancode, action, mods);
}

int main() {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
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
    setupFBO();
    ShadowMapDir_Init();

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
    for (auto& obj : scene) {
    if (obj.collider != nullptr) {
        delete obj.collider;
        obj.collider = nullptr;
    }
    }
    glfwTerminate();
    return 0;
}