#include "skybox.h"
#include "utilities.hpp"
#include "stb_image.h"

Skybox::Skybox(const std::vector<std::string>& faces, const char* vertexPath, const char* fragmentPath) {
    // 1. Tworzenie shaderów
    idProgram = glCreateProgram();
    glAttachShader(idProgram, LoadShader(GL_VERTEX_SHADER, vertexPath));
    glAttachShader(idProgram, LoadShader(GL_FRAGMENT_SHADER, fragmentPath));
    LinkAndValidateProgram(idProgram);

    // 2. Geometria
    setupGeometry();

    // 3. Tekstura
    loadCubemap(faces);
}

void Skybox::setupGeometry() {
    GLfloat positions[] = {
        1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,
        1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f
    };

    GLuint indices[] = {
        5, 0, 1, 5, 4, 0, 2, 0, 3, 2, 1, 0, 7, 0, 4, 7, 3, 0,
        3, 6, 2, 3, 7, 6, 1, 2, 6, 1, 6, 5, 4, 5, 6, 4, 6, 7
    };

    glGenVertexArrays(1, &idVAO);
    glGenBuffers(1, &idVBO);
    glGenBuffers(1, &idEBO);

    glBindVertexArray(idVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, idVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Skybox::loadCubemap(const std::vector<std::string>& faces) {
    glGenTextures(1, &idTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, idTexture);

    stbi_set_flip_vertically_on_load(false);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, 
                         width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cout << "Skybox tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    stbi_set_flip_vertically_on_load(true);
}

void Skybox::draw(const glm::mat4& matProj, const glm::mat4& matView, float scale) {
    glUseProgram(idProgram);

    // 1. Ignorujemy translację kamery (Skybox zawsze wokół nas)
    glm::mat4 viewStatic = glm::mat4(glm::mat3(matView)); 
    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(scale));
    glm::mat4 matPVM = matProj * viewStatic * glm::mat4(1.0f);

    glUniformMatrix4fv(glGetUniformLocation(idProgram, "matPVM"), 1, GL_FALSE, glm::value_ptr(matPVM));

    // 2. Aktywacja tekstury
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, idTexture);
    glUniform1i(glGetUniformLocation(idProgram, "tex_skybox"), 0);

    // 3. Rysowanie bez zapisu do Depth Buffer (żeby skybox nie zasłaniał modeli)
    glDepthMask(GL_FALSE);
    glBindVertexArray(idVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);

    glUseProgram(0);
}

Skybox::~Skybox() {
    glDeleteVertexArrays(1, &idVAO);
    glDeleteBuffers(1, &idVBO);
    glDeleteBuffers(1, &idEBO);
    glDeleteTextures(1, &idTexture);
    glDeleteProgram(idProgram);
}