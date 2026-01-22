#ifndef GROUND_H
#define GROUND_H

#include <vector>
#include <glm/glm.hpp>

extern float GetHeight(float x, float z);

class CGround {
public:
    std::vector<glm::vec3> vertices;

    //wersja ground jako obj
    void Init() {} 

    float getY(glm::vec2 pos) {
        return GetHeight(pos.x, pos.y);
    }

private:
    // Funkcja pomocnicza do sprawdzania czy punkt jest w trójkącie (2D)
    bool isPointInTriangle(glm::vec2 p, glm::vec2 a, glm::vec2 b, glm::vec2 c) {
        float area = 0.5f * (-b.y * c.x + a.y * (-b.x + c.x) + a.x * (b.y - c.y) + b.x * c.y);
        float s = 1.0f / (2.0f * area) * (a.y * c.x - a.x * c.y + (c.y - a.y) * p.x + (a.x - c.x) * p.y);
        float t = 1.0f / (2.0f * area) * (a.x * b.y - a.y * b.x + (a.y - b.y) * p.x + (b.x - a.x) * p.y);
        return s >= 0 && t >= 0 && (s + t) <= 1;
    }
};

#endif