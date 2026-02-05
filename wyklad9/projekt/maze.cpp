#include <vector>
#include <stack>
#include <algorithm>
#include <ctime>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "utilities.hpp"
#include "objloader.hpp"
#include "SceneManager.hpp"
#include "collider.hpp"

struct Point { int x, y; };

const int SIZE = 21;
int maze[SIZE][SIZE];

// Zmienne globalne do instancjonowania
std::vector<glm::mat4> matrices[16]; 

// --- FUNKCJA GENERUJĄCA LOGIKĘ LABIRYNTU ---
void generateMaze() {
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            maze[i][j] = 1;

    std::srand(std::time(0));
    int startX = 1, startY = 1;
    std::stack<Point> stack;
    stack.push({startX, startY});
    maze[startY][startX] = 0;

    while (!stack.empty()) {
        Point curr = stack.top();
        std::vector<Point> neighbors;
        int dx[] = {0, 0, 2, -2}, dy[] = {2, -2, 0, 0};

        for (int i = 0; i < 4; i++) {
            int nx = curr.x + dx[i], ny = curr.y + dy[i];
            if (nx > 0 && nx < SIZE - 1 && ny > 0 && ny < SIZE - 1 && maze[ny][nx] == 1)
                neighbors.push_back({nx, ny});
        }

        if (!neighbors.empty()) {
            Point next = neighbors[std::rand() % neighbors.size()];
            maze[curr.y + (next.y - curr.y) / 2][curr.x + (next.x - curr.x) / 2] = 0;
            maze[next.y][next.x] = 0;
            stack.push(next);
        } else stack.pop();
    }

    maze[SIZE - 2][SIZE - 2] = 2;
}

// --- ANALIZA I TWORZENIE GEOMETRII/COLLIDERÓW ---
void analyzeMaze(std::vector<CMesh>& meshes, float scale) {
    // Czyścimy kontenery przed budową
    for(int i = 0; i < 16; i++) matrices[i].clear();
    
    // activeColliders jest zdefiniowane w collider.hpp, czyścimy je:
    activeColliders.clear();

    for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++) {
            // --- LOGIKA WYJŚCIA (2) ---
            if (maze[y][x] == 2) {
                glm::vec3 exitPos = glm::vec3(x * scale, 0.0f, y * scale);
                // Dodajemy TRIGGER (nie blokuje ruchu, ale go wykrywamy)
                // Ustawiamy wysokość na 1.0f i rozmiar pudła na scale
                // addTriggerCollider(exitPos + glm::vec3(0, 1.0f, 0), glm::vec3(scale/2.0f, 2.0f, scale/2.0f));
                
                // Opcjonalnie: możesz tu dodać macierz do matrices[X] 
                // jeśli masz model schodów lub bramy wyjściowej
                continue; 
            }

            if (maze[y][x] != 1) continue;

            glm::vec3 center = glm::vec3(x * scale, 0.0f, y * scale);
            
            bool u = (y > 0) && (maze[y - 1][x] == 1);
            bool d = (y < SIZE - 1) && (maze[y + 1][x] == 1);
            bool l = (x > 0) && (maze[y][x - 1] == 1);
            bool r = (x < SIZE - 1) && (maze[y][x + 1] == 1);

            // 1. Środek (Słupek)
            glm::vec3 pillarHalf = meshes[6].calculateHalfSizes();
            // UWAGA: Wywołujemy addWallCollider z collider.hpp (bez 'new'!)
            addWallCollider(center + glm::vec3(0, 1.0f, 0), pillarHalf * 2.0f);

            // 2. Ramiona (Ściany łączące)
            float armLen = scale / 2.0f;
            float offset = scale / 4.0f;

            auto addArm = [&](glm::vec3 posOffset, float angleY, bool horizontal) {
                glm::mat4 m = glm::translate(glm::mat4(1.0f), center + posOffset);
                m = glm::rotate(m, glm::radians(angleY), glm::vec3(0, 1, 0));
                matrices[5].push_back(m); // Zapisujemy do instancjonowania wall_1

                glm::vec3 currentSize = horizontal ? glm::vec3(armLen, 2.0f, 0.4f) : glm::vec3(0.4f, 2.0f, armLen);
                addWallCollider(center + posOffset + glm::vec3(0, 1.0f, 0), currentSize);
            };

            if (u) addArm(glm::vec3(0, 0, -offset), 0.0f, false);
            if (d) addArm(glm::vec3(0, 0, offset), 0.0f, false);
            if (l) addArm(glm::vec3(-offset, 0, 0), 90.0f, true);
            if (r) addArm(glm::vec3(offset, 0, 0), 90.0f, true);
        }
    }

    // Finalizacja instancjonowania dla modelu ściany (indeks 5)
    if (!matrices[5].empty()) {
        std::vector<glm::vec3> wallColors(matrices[5].size(), glm::vec3(1.0f));
        meshes[5].PrepareInstancing(matrices[5], wallColors);
    }
}

// Funkcja spinająca, którą wywołasz raz po wczytaniu modeli
void setupMazeInstancing(std::vector<CMesh>& meshes) {
    analyzeMaze(meshes, 2.0f);
}