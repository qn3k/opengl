// ---------------------------------------------------
// Funkcje pomocnicze, w szczegolnosci do wczytywania
// plikow shaderow
// ---------------------------------------------------
#ifndef __UTILITIES_HPP
#define __UTILITIES_HPP


// Zmienne do kontroli obrotu kamery
GLfloat CameraRotate_x = 0.0f;
GLfloat CameraRotate_y = 0.0f;

// Zmienne do kontroli pozycji kamery
GLfloat CameraTranslate_x = 0.0f;
GLfloat CameraTranslate_y = 0.0f;
GLfloat CameraTranslate_z = -2.0f;

// Zmienne do obslugi myszy
int _mouse_buttonState = GLUT_UP;
int _mouse_buttonX;
int _mouse_buttonY;


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
// Funkcja wywolywana przy nacisnieciu tzw. specjalnych
// klawiszy (klawiszy spoza tablicy ASCII)
// --------------------------------------------------------------
void SpecialKeys( int key, int x, int y )
{
    switch( key )
    {

        case GLUT_KEY_LEFT:
            CameraTranslate_x -= 0.2f;
            break;

        case GLUT_KEY_RIGHT:
            CameraTranslate_x += 0.2f;
            break;

		 case GLUT_KEY_UP:
            CameraTranslate_y += 0.2f;
            break;

        case GLUT_KEY_DOWN:
            CameraTranslate_y -= 0.2f;
            break;

    }

    glutPostRedisplay();
}



// --------------------------------------------------------------
// Funkcja zwrotna wywolywana podczas nacisnieciu klawisza myszy
// --------------------------------------------------------------
void MouseButton( int button, int state, int x, int y )
{
    if( button == GLUT_LEFT_BUTTON )
    {

        _mouse_buttonState = state;


        if( state == GLUT_DOWN )
        {
            _mouse_buttonX = x;
            _mouse_buttonY = y;
        }
    }
}

// --------------------------------------------------------------
// Funkcja zwrotna wywolywana podczas ruchu kursorem myszy
// --------------------------------------------------------------
void MouseMotion( int x, int y )
{
    if( _mouse_buttonState == GLUT_DOWN )
    {
        CameraRotate_y += 2*(x - _mouse_buttonX)/(float)glutGet( GLUT_WINDOW_WIDTH );
        _mouse_buttonX = x;
        CameraRotate_x -= 2*(_mouse_buttonY - y)/(float)glutGet( GLUT_WINDOW_HEIGHT );
        _mouse_buttonY = y;
        glutPostRedisplay();
    }
}

// --------------------------------------------------------------
// Funkcja wywolywana podczas ruchu rolki myszy
// --------------------------------------------------------------
void MouseWheel(int button, int dir, int x, int y)
{
    if (dir > 0)
    {
        // Zoom in
        CameraTranslate_z += 0.5f;
    }
    else
    {
        // Zoom out
        CameraTranslate_z -= 0.5f;
    }

    glutPostRedisplay();
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

