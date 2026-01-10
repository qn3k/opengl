// ---------------------------------------------------
// Funkcje pomocnicze, w szczegolnosci do wczytywania
// plikow shaderow
// ---------------------------------------------------
#ifndef __UTILITIES_HPP
#define __UTILITIES_HPP

// ---------------------------------------------------
// Makro do znajdowania prostych bledow opengla
// ---------------------------------------------------
#define __CHECK_FOR_ERRORS 	{GLenum errCode; if ((errCode = glGetError()) != GL_NO_ERROR) printf("Error code %d in the file %s at line %d !\n", errCode, __FILE__,  __LINE__);}




// Wymiary okna
int windowWidth = 800, windowHeight = 600;

// Zmienne do kontroli obrotu kamery
GLfloat CameraRotate_x = 0.0f;
GLfloat CameraRotate_y = 0.0f;

// Zmienne do kontroli pozycji kamery
GLfloat CameraTranslate_x = 0.0f;
GLfloat CameraTranslate_y = 0.0f;
GLfloat CameraTranslate_z = -2.0f;

// Zmienne do obslugi myszy
double __mouse_buttonX, __mouse_buttonY;
bool Mouse_Press = false;
int  Mouse_Button = 0;

// --------------------------------------------------------------
// Funkcja zwraca macierz widoku dla kamery
// --------------------------------------------------------------
glm::mat4 UpdateViewMatrix()
{
	glm::mat4 matView = glm::mat4x4( 1.0 );
	matView = glm::translate( matView, glm::vec3( CameraTranslate_x, CameraTranslate_y, CameraTranslate_z) );
	matView = glm::rotate( matView, CameraRotate_x, glm::vec3( 1.0f, 0.0f, 0.0f ) );
	matView = glm::rotate( matView, CameraRotate_y, glm::vec3( 0.0f, 1.0f, 0.0f ) );

	return matView;
}


// --------------------------------------------------------------
// NOWE: funkcja wyliczajaca pozycje kamery z macierzy widoku
// zrodlo: https://community.khronos.org/t/extracting-camera-position-from-a-modelview-matrix/68031
// --------------------------------------------------------------
glm::vec3 ExtractCameraPos(const glm::mat4 & a_modelView)
{
  // Get the 3 basis vector planes at the camera origin and transform them into model space.
  //
  // NOTE: Planes have to be transformed by the inverse transpose of a matrix
  //       Nice reference here: http://www.opengl.org/discussion_boards/showthread.php/159564-Clever-way-to-transform-plane-by-matrix
  //
  //       So for a transform to model space we need to do:
  //            inverse(transpose(inverse(MV)))
  //       This equals : transpose(MV) - see Lemma 5 in http://mathrefresher.blogspot.com.au/2007/06/transpose-of-matrix.html
  //
  // As each plane is simply (1,0,0,0), (0,1,0,0), (0,0,1,0) we can pull the data directly from the transpose matrix.
  //
  glm::mat4 modelViewT = transpose(a_modelView);

  // Get plane normals
  glm::vec3 n1(modelViewT[0]);
  glm::vec3 n2(modelViewT[1]);
  glm::vec3 n3(modelViewT[2]);

  // Get plane distances
  float d1(modelViewT[0].w);
  float d2(modelViewT[1].w);
  float d3(modelViewT[2].w);

  // Get the intersection of these 3 planes
  // http://paulbourke.net/geometry/3planes/
  glm::vec3 n2n3 = cross(n2, n3);
  glm::vec3 n3n1 = cross(n3, n1);
  glm::vec3 n1n2 = cross(n1, n2);

  glm::vec3 top = (n2n3 * d1) + (n3n1 * d2) + (n1n2 * d3);
  float denom = dot(n1, n2n3);

  return top / -denom;
}


// ---------------------------------------------------
// funkcja zwrotna do obslugi zmiany rozmiaru framebuffera
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    windowWidth = width;
    windowHeight = height;

    // Aktualizacja macierzy rzutowania
    if (windowHeight != 0)
        matProj = glm::perspective(glm::radians(80.0f), windowWidth/(float)windowHeight, 0.1f, 50.0f);

    // Viewport
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
}



// ---------------------------------------------------
// funkcja zwrotna do obslugi scrolla myszy
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    CameraTranslate_z += yoffset;
}

// --------------------------------------------------------------
// funkcja zwrotna do obslugi ruchu kursora myszy
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{

	if (Mouse_Press && Mouse_Button == GLFW_MOUSE_BUTTON_LEFT)
	{
		CameraRotate_x += (ypos - __mouse_buttonY)/(float)windowWidth;
		CameraRotate_y += (xpos - __mouse_buttonX)/(float)windowHeight;
        __mouse_buttonX = xpos;
        __mouse_buttonY = ypos;
	}

	if (Mouse_Press && Mouse_Button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		CameraRotate_x -= 2*(ypos - __mouse_buttonY)/(float)windowWidth;
		CameraRotate_y += 2*(__mouse_buttonX - xpos)/(float)windowHeight;
        __mouse_buttonX = xpos;
        __mouse_buttonY = ypos;
	}
}


// --------------------------------------------------------------
// funkcja zwrotna do obslugi klawiszy myszy
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (action == GLFW_PRESS)
	{
		Mouse_Press = true;
		Mouse_Button = button;

		glfwGetCursorPos(window, &__mouse_buttonX, &__mouse_buttonY);
	}

	if (action == GLFW_RELEASE)
	{
		Mouse_Press = false;
	}
}

// ---------------------------------------------------
// funkcja zwrotna do obslugi klawiatury
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (action == GLFW_PRESS)
    {
        printf("Nacisnieto klawisz %d \n", key);
    }

}


// ---------------------------------------------------
// funkcja zwrotna do obslugi bledow glfw
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);

}

// ---------------------------------------------
unsigned long getFileLength(const char *filename)
{
	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		fprintf(stderr, "Can't open the file %s!\n", filename);
		exit(1);
	}

	unsigned long fileSize;

	fseek(file, 0, SEEK_END);
	fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);
	fclose(file);

	return fileSize;
}



// ---------------------------------------------
GLchar * LoadShaderFile(const char* filename)
{
	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		fprintf(stderr, "Nie moge otworzyc pliku %s!\n", filename);
		exit(1);
	}

	unsigned long fileSize;

	fseek(file, 0, SEEK_END);
	fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	if (fileSize == 0) {
		printf("Plik %s jest pusty!\n", filename);
		exit(1);
	};

	GLchar *ShaderSource = new GLchar[fileSize + 1];
	if (ShaderSource == NULL) {
		printf("Nie moge zaalokowac %ld bajtow \n", fileSize + 1);
		exit(1);
	}

	int bytesRead = 0;
	char c;
	while ((c = fgetc(file)) != EOF && bytesRead < fileSize - 1) {
		ShaderSource[bytesRead++] = c;
	}
	ShaderSource[bytesRead] = '\0';

	fclose(file);
	return ShaderSource;
}

// ---------------------------------------
void CheckForErrors_Shader(GLuint shader)
{
	GLint status;
	glGetShaderiv( shader, GL_COMPILE_STATUS, &status );
	if( status != GL_TRUE )
	{
		printf("OpenGL Error!\n");
		GLint logLength;
		glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &logLength );
		char *log = new char[logLength];
		glGetShaderInfoLog( shader, logLength, NULL, log );
		printf("LOG: %s\n", log);
		delete[] log;
	}
}

// ---------------------------------------
void CheckForErrors_Program(GLuint program, GLenum mode)
{
	GLint status;
	glGetProgramiv( program, mode, &status );
	if( status != GL_TRUE )
	{
		switch (mode)
		{
			case GL_LINK_STATUS:
				printf("Blad konsolidacji programu!\n");
				break;
			case GL_VALIDATE_STATUS:
				printf("Blad walidacji programu!\n");
				break;
			default:
				printf("Inny blad programu!\n");

		}
		GLint logLength;
		glGetProgramiv( program, GL_INFO_LOG_LENGTH, &logLength );
		char *log = new char[logLength];
		glGetProgramInfoLog( program, logLength, NULL, log );
		printf("LOG: %s\n", log);
		delete[] log;
	}
}

// ---------------------------------------
GLuint LoadShader( GLuint MODE, const char *filename  )
{
	// utworzenie obiektu shadera
	GLuint shader = glCreateShader( MODE );

	// Wczytanie kodu shadera z pliku
	GLchar *code = LoadShaderFile(filename);

	glShaderSource( shader, 1, &code, NULL );
	glCompileShader( shader );
	CheckForErrors_Shader(shader);

	delete[] code;
	return shader;
}

// ---------------------------------------
void LinkAndValidateProgram(GLuint program)
{
	// Konsolidacja programu
	glLinkProgram( program );
	CheckForErrors_Program(program, GL_LINK_STATUS);

	// Walidacja programu
	glValidateProgram( program );
	CheckForErrors_Program(program, GL_VALIDATE_STATUS);
}

#endif

