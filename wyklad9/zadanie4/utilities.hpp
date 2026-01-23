#ifndef __UTILITIES_HPP
#define __UTILITIES_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>

// Deklaracje zmiennych globalnych (zdefiniowanych w main.cpp)
extern int windowWidth, windowHeight;
extern const char* windowTitle;
extern glm::mat4 matProj;

// Deklaracje zmiennych sterowania (zdefiniowanych w utilities.cpp)
extern float CameraRotate_x, CameraRotate_y;
extern float CameraTranslate_x, CameraTranslate_y, CameraTranslate_z;

// Prototypy funkcji 
glm::mat4 UpdateViewMatrix();
glm::vec3 ExtractCameraPos(const glm::mat4 & a_modelView);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void error_callback(int error, const char* description);

unsigned long getFileLength(const char *filename);
GLchar* LoadShaderFile(const char* filename);
void CheckForErrors_Shader(GLuint shader);
void CheckForErrors_Program(GLuint program, GLenum mode);
GLuint LoadShader(GLuint MODE, const char *filename);
void LinkAndValidateProgram(GLuint program);

#endif