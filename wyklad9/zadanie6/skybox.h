#ifndef SKYBOX_H
#define SKYBOX_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>
#include <iostream>

class Skybox {
private:
    GLuint idVAO;
    GLuint idVBO;
    GLuint idEBO;
    GLuint idProgram;
    GLuint idTexture;

    void loadCubemap(const std::vector<std::string>& faces);
    void setupGeometry();

public:
    // Konstruktor przyjmuje ścieżki do 6 tekstur
    Skybox(const std::vector<std::string>& faces, const char* vertexPath, const char* fragmentPath);
    
    // Destruktor czyści pamięć w GPU
    ~Skybox();

    // Metoda renderująca
    void draw(const glm::mat4& matProj, const glm::mat4& matView, float scale = 100.0f);

    GLuint getTextureID() const { return idTexture; }
};

#endif