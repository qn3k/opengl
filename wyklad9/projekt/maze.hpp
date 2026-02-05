#ifndef MAZE_HPP
#define MAZE_HPP

#include <vector>
#include <glm/glm.hpp>
#include "objloader.hpp" // Zakładam, że tu jest definicja CMesh

// Struktura pomocnicza dla współrzędnych siatki
struct Point { 
    int x, y; 
};

// Stałe wymiary labiryntu
const int SIZE = 21;

extern int maze[SIZE][SIZE];

extern std::vector<glm::mat4> matrices[16];

void generateMaze();

void analyzeMaze(std::vector<CMesh>& meshes, float scale);

void setupMazeInstancing(std::vector<CMesh>& meshes);

#endif // MAZE_HPP