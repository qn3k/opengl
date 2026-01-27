#include "PlayerLogic.hpp"
//#include "SceneManager.hpp"
#include <cmath>

extern float GetHeight(float x, float z);

//sterowanie postacia
void handleInput(bool* keys, CPlayer& myPlayer, std::vector<SceneObject>& scene, 
                 const std::vector<CMesh>& meshes, const PlayerIndices& pIdx, 
                 int playerIdx, float playerRadius, const std::vector<glm::mat4>& treeMatrices) {
    float speed = 0.05f;
    float rotSpeed = 0.03f;
    float angleOffset = 1.57f;

    // 1. Obrót (zawsze dozwolony)
    if (keys[GLFW_KEY_A]) myPlayer.Rotate(rotSpeed);
    if (keys[GLFW_KEY_D]) myPlayer.Rotate(-rotSpeed);

    // 2. Obliczanie potencjalnej nowej pozycji
    glm::vec3 nextPos = myPlayer.position;
    bool moving = false;

    if (keys[GLFW_KEY_W]) {
        nextPos.x += speed * sin(myPlayer.rotationY + angleOffset);
        nextPos.z += speed * cos(myPlayer.rotationY + angleOffset);
        moving = true;
    }
    if (keys[GLFW_KEY_S]) {
        nextPos.x -= speed * sin(myPlayer.rotationY + angleOffset);
        nextPos.z -= speed * cos(myPlayer.rotationY + angleOffset);
        moving = true;
    }

    if (moving) {
        bool collision = false;

        // A. Granice mapy
        float terrainHeight = GetHeight(nextPos.x, nextPos.z);
        if (terrainHeight < -1000.0f) { 
            collision = true;
        }

        // B. Kolizje z obiektami ze sceny
        for (int i = 0; i < scene.size(); i++) {
            const auto& obj = scene[i];

            // 1. FILTROWANIE: Ignoruj podłogę i WSZYSTKIE części gracza
            if (obj.mesh == &meshes[0]) continue; 
            if (i == pIdx.body || i == pIdx.legR || i == pIdx.legL || i == playerIdx) continue;

            // Jeśli obiekt jest znacznie wyżej niż głowa gracza (np. powyżej 5.0f), ignoruj kolizję
            if (obj.position.y > (nextPos.y + 5.0f)) {
                continue; 
            }
            
            // Opcjonalnie: jeśli obiekt jest głęboko pod ziemią, też go ignoruj
            /*
            if (obj.position.y < (nextPos.y - 2.0f)) {
                continue;
            }
            */

            // 2. OBLICZENIE DYSTANSU
            float dist = glm::distance(glm::vec2(nextPos.x, nextPos.z), glm::vec2(obj.position.x, obj.position.z));
            
            // 3. DYNAMICZNY PROMIEŃ
            float objRadius = obj.scale.x; 
            if (objRadius > 5.0f) objRadius = 2.0f; // Zabezpieczenie przed gigantycznymi obiektami

            float minDistance = playerRadius + (objRadius * 0.5f); 
            
            if (dist < minDistance) {
                collision = true;
                break;
            }
        }

        // C. Kolizje z drzewami (treeMatrices)
        if (!collision) {
            for (const auto& treeMat : treeMatrices) {
                glm::vec3 treePos = glm::vec3(treeMat[3]); 
                float dist = glm::distance(glm::vec2(nextPos.x, nextPos.z), glm::vec2(treePos.x, treePos.z));
                if (dist < (playerRadius + 0.3f)) {
                    collision = true;
                    break;
                }
            }
        }

        if (!collision) {
            myPlayer.position = nextPos;
            myPlayer.position.y = terrainHeight;
        }
    }
}

void playerAnimation(const bool* keys, CPlayer& myPlayer, std::vector<SceneObject>& scene, const PlayerIndices& pIdx) {
    if (pIdx.body != -1 && pIdx.legR != -1 && pIdx.legL != -1) {

        float t = (float)glfwGetTime();
        
        // Sprawdzamy czy gracz trzyma klawisze ruchu
        bool isMoving = keys[GLFW_KEY_W] || keys[GLFW_KEY_S];
        
        // Animacje pomocnicze
        float bobbing = isMoving ? abs(sin(t * 16.0f)) * 0.05f : 0.0f;
        float walkAngle = isMoving ? sin(t * 8.0f) * 0.6f : 0.0f;

        glm::vec3 offset = glm::vec3(0.0f, 0.3f + bobbing, 0.0f); 
        glm::vec3 p = myPlayer.position + offset;
        float rotY = myPlayer.rotationY;

        // Aktualizacja transformacji w scene[]
        scene[pIdx.body].position = p;
        scene[pIdx.legR].position = p;
        scene[pIdx.legL].position = p;

        scene[pIdx.body].rotation = glm::vec3(0.0f, rotY, 0.0f);
        scene[pIdx.legR].rotation = glm::vec3(0.0f, rotY, walkAngle); 
        scene[pIdx.legL].rotation = glm::vec3(0.0f, rotY, -walkAngle);
    }
}