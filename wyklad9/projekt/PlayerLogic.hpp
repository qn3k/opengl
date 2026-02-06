#ifndef PLAYER_LOGIC_HPP
#define PLAYER_LOGIC_HPP

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "player.hpp"
#include "SceneManager.hpp" 
#include "collider.hpp"

void handleInput(
    bool* keys, 
    CPlayer& myPlayer, 
    std::vector<SceneObject>& scene, 
    std::vector<CMesh>& meshes, // USUNIĘTO 'const'
    const PlayerIndices& pIdx, 
    int playerIdx, 
    float playerRadius
);

void playerAnimation(
    const bool* keys, 
    CPlayer& myPlayer, 
    std::vector<SceneObject>& scene, 
    const PlayerIndices& pIdx
);

#endif