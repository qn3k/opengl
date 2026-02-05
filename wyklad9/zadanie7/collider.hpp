#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <cstdio> 

//typy kolizji
enum class ColliderType {
WALL,       // Zwykła przeszkoda (blokuje ruch)
TRIGGER,    // Np. wejście do nowej strefy (nie blokuje, ale wyzwala akcję)
DAMAGE,     // Kolczatka/Lawa (zadaje obrażenia)
COLLECTIBLE // Moneta/Przedmiot
};

// ----------------------------------------------------------------
// Klasa abstrakcyjna obiektu kolizyjnego
// ----------------------------------------------------------------
class CCollider
{
public:
    ColliderType Type;

    CCollider(ColliderType type) : Type(type) {}
    virtual ~CCollider() = default;

    virtual bool isCollision(const CCollider * other) const = 0;
};

// ----------------------------------------------------------------
// Kolider Sferyczny
// ----------------------------------------------------------------
class CSphereCollider : public CCollider
{
public:
    glm::vec3 Position; 
    float Radius;

    CSphereCollider(const glm::vec3& pos, float r, ColliderType type = ColliderType::WALL)
    : CCollider(type), Position(pos), Radius(r) 
    {}

    virtual bool isCollision(const CCollider *_other) const override
    {
        const CSphereCollider *otherSphere = dynamic_cast<const CSphereCollider*>(_other);
        if (otherSphere) {
            float distance = glm::distance(this->Position, otherSphere->Position);
            return distance < (this->Radius + otherSphere->Radius);
        }

        return _other->isCollision(this);
    }
};

// ----------------------------------------------------------------
// Axis Aligned Bounding Box (AABB)
// ----------------------------------------------------------------
class CAABBCollider : public CCollider
{
public:
    glm::vec3 Position; 
    glm::vec3 HalfSizes; // Zamiast float Length, używamy wektora

    // Konstruktor przyjmujący wektor wymiarów
    CAABBCollider(const glm::vec3& pos, const glm::vec3& halfSizes, ColliderType type = ColliderType::WALL)
    : CCollider(type), Position(pos), HalfSizes(halfSizes) 
    {}

    virtual bool isCollision(const CCollider *_other) const override
    {
        const CAABBCollider *otherBox = dynamic_cast<const CAABBCollider*>(_other);
        if (otherBox) {
            glm::vec3 minA = get_min();
            glm::vec3 maxA = get_max();
            glm::vec3 minB = otherBox->get_min();
            glm::vec3 maxB = otherBox->get_max();
            return (minA.x <= maxB.x && maxA.x >= minB.x) &&
                   (minA.y <= maxB.y && maxA.y >= minB.y) &&
                   (minA.z <= maxB.z && maxA.z >= minB.z);
        }

        const CSphereCollider *sphere = dynamic_cast<const CSphereCollider*>(_other);
        if (sphere) {
            glm::vec3 minBound = get_min();
            glm::vec3 maxBound = get_max();

            float closestX = std::max(minBound.x, std::min(sphere->Position.x, maxBound.x));
            float closestY = std::max(minBound.y, std::min(sphere->Position.y, maxBound.y));
            float closestZ = std::max(minBound.z, std::min(sphere->Position.z, maxBound.z));

            float distance = glm::distance(glm::vec3(closestX, closestY, closestZ), sphere->Position);
            return distance < sphere->Radius;
        }
        return false;
    }

    // Min i Max uwzględniają teraz różne wymiary dla każdej osi
    inline glm::vec3 get_min() const { return Position - HalfSizes; }
    inline glm::vec3 get_max() const { return Position + HalfSizes; }
};