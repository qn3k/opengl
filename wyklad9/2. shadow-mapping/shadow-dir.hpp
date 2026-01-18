// -------------------------------------------------
// Programowanie grafiki 3D w OpenGL / UG
// -------------------------------------------------
#ifndef __SHADOW_DIR_H
#define __SHADOW_DIR_H

// Identyfikatory obiektow
GLuint DepthMap_idProgram;
GLuint DepthMap_idFrameBuffer;
GLuint DepthMap_idTexture;

// Rozdzielczosc depth mapy
const unsigned int DepthMap_Width = 1024;
const unsigned int DepthMap_Height = 1024;


// Uproszczone parametry swiatla kierunkowego
// Znacznie lepiej ubrac to w strukture,
// ktora uzywalismy podczas zajec z oswietlenia
glm::vec3 lightDirection;
glm::vec3 lightPosition;

// Macierze rzutowania dla kamery patrzacej z punktu
// widzenia oswietlenia
glm::mat4 lightProj;
glm::mat4 lightView;

// -------------------------------------------------------
void ShadowMapDir_Init()
{
	// Ustawienie polozenia i kierunku swiatla
	lightDirection = glm::normalize(glm::vec3(0.2, -0.8f, 1.1f));
	lightPosition = glm::vec3(0.0f, 5.0f, -25.0f);

	// Ustawienie macierzy kamery swiatla
	lightProj = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 2.0f, 30.5f);
	lightView = glm::lookAt(lightPosition, lightPosition + lightDirection, glm::vec3(0.0f, 1.0f, 0.0f));


	// 1. Stworzenie obiektu tekstury
	glGenTextures(1, &DepthMap_idTexture);
	glBindTexture(GL_TEXTURE_2D, DepthMap_idTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, DepthMap_Width, DepthMap_Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// 2. Stworzenie obiektu ramki bufora jedynie z buforem glebokosci
	// (skladowa koloru nie jest nam potrzebna)
	glGenFramebuffers(1, &DepthMap_idFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, DepthMap_idFrameBuffer);
		// 3. Dolaczanie tekstury do ramki bufora
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthMap_idTexture, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// 4. Stworzenie oddzielnego/uproszczonego programu,
	// ktory bedzie generowal mape cieni
	DepthMap_idProgram = glCreateProgram();
		glAttachShader( DepthMap_idProgram, LoadShader(GL_VERTEX_SHADER, "shaders/depthmap-vertex.glsl"));
		glAttachShader( DepthMap_idProgram, LoadShader(GL_FRAGMENT_SHADER, "shaders/depthmap-fragment.glsl"));
	LinkAndValidateProgram( DepthMap_idProgram );
}

// -------------------------------------------------------
void ShadowMapDir_Clean()
{
	glDeleteProgram(DepthMap_idProgram);
	glDeleteTextures(1, &DepthMap_idTexture);
	glDeleteFramebuffers(1, &DepthMap_idFrameBuffer);
}


#endif // __SHADOW_DIR_H
