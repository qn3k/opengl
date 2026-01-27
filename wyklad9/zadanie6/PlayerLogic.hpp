#ifndef PLAYER_LOGIC_HPP
#define PLAYER_LOGIC_HPP

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "player.hpp"
#include "SceneManager.hpp" 

void handleInput(
    bool* keys, 
    CPlayer& myPlayer, 
    std::vector<SceneObject>& scene, 
    const std::vector<CMesh>& meshes,
    const PlayerIndices& pIdx,
    int playerIdx,
    float playerRadius,
    const std::vector<glm::mat4>& treeMatrices
);

void playerAnimation(
    const bool* keys, 
    CPlayer& myPlayer, 
    std::vector<SceneObject>& scene, 
    const PlayerIndices& pIdx
);

#endif