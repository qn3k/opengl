#ifndef CTEXTURE_HPP
#define CTEXTURE_HPP

#include <glad/glad.h>
#include "stb_image.h" 

class CTexture {
public:
    GLuint idTexture;

    CTexture() : idTexture(0) {}

    // Metoda wczytujaca teksture z pliku
    bool Load(const char* path) {
        int width, height, nrChannels;
        // Wczytanie obrazu za pomoca stb_image
        //stbi_set_flip_vertically_on_load(true); // Opcjonalnie: odwraca teksture w pionie
        unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);

        if (data) {
            GLenum format = GL_RGB;
            if (nrChannels == 1)
                format = GL_RED;
            else if (nrChannels == 3)
                format = GL_RGB;
            else if (nrChannels == 4)
                format = GL_RGBA;

            glGenTextures(1, &idTexture);
            glBindTexture(GL_TEXTURE_2D, idTexture);

            // Ustawienia filtrowania i zawijania
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // Wczytanie danych do obiektu tekstury
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            stbi_image_free(data); // Zwolnienie pamieci obrazu
            printf("Zaladowano teksture: %s\n", path);
            return true;

        } else {
            printf("Nie udalo sie wczytac tekstury: %s\n", path);
            return false;
        }
    }

    void Bind(GLenum textureUnit = GL_TEXTURE0) {
        glActiveTexture(textureUnit);
        glBindTexture(GL_TEXTURE_2D, idTexture);
    }

    void Unbind() {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Release() {
        glDeleteTextures(1, &idTexture);
    }
};

#endif