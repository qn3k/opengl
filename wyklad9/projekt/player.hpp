#ifndef PLAYER_H
#define PLAYER_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "ground.hpp"
#include "collider.hpp"

class CPlayer {
public:
    glm::vec3 position;
    float rotationY;
    CGround* ground;
    CSphereCollider* collider;

    void Init(CGround* g) {
        ground = g;
        position = glm::vec3(0, 0, 0);
        rotationY = 0.0f;
        collider = new CSphereCollider(this->position, 1.0f, ColliderType::WALL);
    }

    void Move(float speed) {
        // Ruch w kierunku, w którym patrzy postać
        position.x += speed * cos(rotationY); 
        position.z -= speed * sin(rotationY);   
        // Aktualizacja wysokości na podstawie terenu
        position.y = ground->getY(glm::vec2(position.x, position.z));
    }

    void Rotate(float angle) { rotationY += angle; }
};
#endif