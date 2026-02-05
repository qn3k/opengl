#include "PlayerLogic.hpp"
#include <cmath>
#include <algorithm>

extern float GetHeight(float x, float z);
extern std::vector<glm::mat4> treeMatrices;

// 1. Logika fizyki 
bool checkGlobalCollisions(const glm::vec3& potentialPos, std::vector<SceneObject>& scene, const PlayerIndices& pIdx) {
    CSphereCollider playerTest(potentialPos, 0.5f, ColliderType::WALL); 
    bool movementBlocked = false;

    // A. Obiekty ze sceny
    for (size_t i = 0; i < scene.size(); ++i) {
        if (scene[i].isCollected) continue;
        if ((int)i == pIdx.body || (int)i == pIdx.legR || (int)i == pIdx.legL) continue;

        if (scene[i].collider != nullptr) {
            if (scene[i].collider->isCollision(&playerTest)) {
                
                // --- LOGIKA TYPÓW KOLIZJI ---
                switch (scene[i].collider->Type) {
                    case ColliderType::WALL:
                        movementBlocked = true; // Ściana blokuje ruch
                        break;

                    case ColliderType::DAMAGE:
                        // playerHealth -= 0.1f; 
                        movementBlocked = true; // Może też blokować przejście
                        break;

                    case ColliderType::COLLECTIBLE:
                        // Jeśli to moneta, "usuń" ją ze świata
                        scene[i].isCollected = true;
                        printf("Zebrałeś przedmiot!\n");
                        // Przedmiot nie blokuje ruchu
                        break;

                    case ColliderType::TRIGGER:
                        //default
                        break;
                }
            }
        }
    }

    // B. Drzewa (Instanced Rendering)
    for (const auto& mat : treeMatrices) {
        // mat[3] to czwarta kolumna macierzy (translacja)
        glm::vec3 treePos = glm::vec3(mat[3]); 
        CSphereCollider treeCollider(treePos, 0.4f, ColliderType::WALL);
        
        if (treeCollider.isCollision(&playerTest)) {
            return true;
        }
    }

    return movementBlocked;
}

// 2. Obsługa wejścia
void handleInput(bool* keys, CPlayer& myPlayer, std::vector<SceneObject>& scene, 
                 const std::vector<CMesh>& meshes, const PlayerIndices& pIdx, 
                 int playerIdx, float playerRadius) {
                 
    float speed = 0.08f;
    float rotSpeed = 0.04f;
    float angleOffset = 1.57f; // Dopasowanie do Twojego modelu OBJ

    // Obrót
    if (keys[GLFW_KEY_A]) myPlayer.Rotate(rotSpeed);
    if (keys[GLFW_KEY_D]) myPlayer.Rotate(-rotSpeed);

    // Obliczanie wektora ruchu
    glm::vec3 moveDir(0.0f);
    bool isMoving = false;

    if (keys[GLFW_KEY_W]) {
        moveDir.x += sin(myPlayer.rotationY + angleOffset);
        moveDir.z += cos(myPlayer.rotationY + angleOffset);
        isMoving = true;
    }
    if (keys[GLFW_KEY_S]) {
        moveDir.x -= sin(myPlayer.rotationY + angleOffset);
        moveDir.z -= cos(myPlayer.rotationY + angleOffset);
        isMoving = true;
    }

    if (isMoving) {
        // Normalizacja, żeby ruch "na skos" nie był szybszy (jeśli dodasz klawisze bocznego ruchu)
        if (glm::length(moveDir) > 0.0f) moveDir = glm::normalize(moveDir);

        glm::vec3 nextPos = myPlayer.position + moveDir * speed;

        // 1. Sprawdzamy wysokość terenu (czy nie wychodzimy poza mapę)
        float terrainHeight = GetHeight(nextPos.x, nextPos.z);
        
        if (terrainHeight > -9000.0f) { // Jeśli jesteśmy na mapie
            nextPos.y = terrainHeight; // Dopasuj wysokość do terenu

            // 2. Sprawdzamy kolizje z obiektami (System Colliderów)
            if (!checkGlobalCollisions(nextPos, scene, pIdx)) {
                myPlayer.position = nextPos;
            }
        }
    }
}

// 3. Animacja (bez zmian w logice, tylko synchronizacja)
void playerAnimation(const bool* keys, CPlayer& myPlayer, std::vector<SceneObject>& scene, const PlayerIndices& pIdx) {
    if (pIdx.body == -1) return;

    float t = (float)glfwGetTime();
    bool isMoving = keys[GLFW_KEY_W] || keys[GLFW_KEY_S];
    
    float bobbing = isMoving ? abs(sin(t * 16.0f)) * 0.05f : 0.0f;
    float walkAngle = isMoving ? sin(t * 10.0f) * 0.5f : 0.0f;

    // Wysokość bazowa ludzika
    glm::vec3 p = myPlayer.position + glm::vec3(0.0f, 0.4f + bobbing, 0.0f);
    float rotY = myPlayer.rotationY;

    // Synchronizacja części ciała z pozycją gracza
    scene[pIdx.body].position = p;
    scene[pIdx.legR].position = p;
    scene[pIdx.legL].position = p;

    scene[pIdx.body].rotation.y = rotY;
    scene[pIdx.legR].rotation.y = rotY;
    scene[pIdx.legL].rotation.y = rotY;

    // Animacja nóg (rotacja w osi lokalnej dla chodu)
    scene[pIdx.legR].rotation.z = walkAngle;
    scene[pIdx.legL].rotation.z = -walkAngle;
}