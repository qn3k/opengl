// ---------------------------------------------------
// Funkcje pomocnicze, w szczegolnosci do wczytywania
// plikow shaderow
// ---------------------------------------------------
#ifndef __UTILITIES_HPP
#define __UTILITIES_HPP


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

	GLchar *ShaderSource = (GLchar *)malloc(sizeof(GLchar) * (fileSize + 1));
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

