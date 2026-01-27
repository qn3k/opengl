    #pragma once
    #include <glad/glad.h>
    #include <glm/glm.hpp>
    #include <vector>
    #include <string>
    #include <cstdio> 

    // ----------------------------------------------------------------
    // Klasa abstrakcyjna obiektu kolizyjnego
    // ----------------------------------------------------------------
    class CCollider
    {
    public:
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

        CSphereCollider(const glm::vec3& pos, float r)
            : Position(pos), Radius(r)
        { }

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
        float Length;

        CAABBCollider (const glm::vec3& pos, float length)
            : Position(pos), Length(length)
        { }

    virtual bool isCollision(const CCollider *_other) const override
    {
        // A. Najpierw sprawdźmy, czy drugi obiekt to Box (to już masz)
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

        // B. DODAJ TO: Sprawdź, czy drugi obiekt to Sfera (Gracz)
        const CSphereCollider *sphere = dynamic_cast<const CSphereCollider*>(_other);
        if (sphere) {
            glm::vec3 minBound = get_min();
            glm::vec3 maxBound = get_max();

            // Znajdź punkt na pudełku najbliższy środkowi sfery
            float closestX = std::max(minBound.x, std::min(sphere->Position.x, maxBound.x));
            float closestY = std::max(minBound.y, std::min(sphere->Position.y, maxBound.y));
            float closestZ = std::max(minBound.z, std::min(sphere->Position.z, maxBound.z));

            // Oblicz dystans między tym punktem a środkiem sfery
            float distance = glm::distance(glm::vec3(closestX, closestY, closestZ), sphere->Position);
            
            // Kolizja zachodzi, gdy dystans jest mniejszy niż promień sfery
            return distance < sphere->Radius;
        }

        return false;
        }

        inline glm::vec3 get_min() const { return Position - glm::vec3(Length); }
        inline glm::vec3 get_max() const { return Position + glm::vec3(Length); }

        bool isPointInsideAABB(glm::vec3 _point) const
        {
            glm::vec3 min = get_min();
            glm::vec3 max = get_max();
            return  (_point.x >= min.x && _point.x <= max.x) &&
                    (_point.y >= min.y && _point.y <= max.y) &&
                    (_point.z >= min.z && _point.z <= max.z);
        }
    };