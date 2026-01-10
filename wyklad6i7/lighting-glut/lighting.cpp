// -------------------------------------------------
// Programowanie grafiki 3D w OpenGL / UG
// -------------------------------------------------
// Przyklad implementacji oswietlenia punktowego
// -------------------------------------------------
#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
// Biblioteki GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Lokalne pliki naglowkowe
#include "utilities.hpp"
#include "objloader.hpp"


// Macierze PVM
glm::mat4 matProj;
glm::mat4 matView;
glm::mat4 matModel = glm::mat4(1.0);


// Identyfikatory obiektow
GLuint idProgram;
GLuint idVAO;

// Wektory na dane z pliku OBJ
std::vector<glm::vec3> OBJ_vertices;
std::vector<glm::vec2> OBJ_uvs;
std::vector<glm::vec3> OBJ_normals;



// ---------------------------------------------------
void DisplayScene()
{
	// Czyszczenie bufora koloru i glebokosci
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// -----------------------------------------------
	// NOWE : makro do wykrywania prostych bledow
	// -----------------------------------------------
	__CHECK_FOR_ERRORS

	// ------------------------------------------------------------
	// Wyliczanie macierz widoku
	// ------------------------------------------------------------
	matView = UpdateViewMatrix();

	// -----------------------------------------------
	// NOWE : wyliczanie pozycji kamery z matView
	// -----------------------------------------------
	glm::vec3 cameraPos = ExtractCameraPos(matView);



	// Wlaczenie programu
	glUseProgram( idProgram );


		// Przekazanie macierzy rzutowania i widoku
		glUniformMatrix4fv( glGetUniformLocation(idProgram, "matProj"), 1, GL_FALSE, glm::value_ptr(matProj));
		glUniformMatrix4fv( glGetUniformLocation(idProgram, "matView"), 1, GL_FALSE, glm::value_ptr(matView));

		// Przekazanie pozycji kamery
		glUniform3fv( glGetUniformLocation( idProgram, "cameraPos" ), 1, &cameraPos[0] );



		// Przekazanie macierzy modelu obiektu
		glUniformMatrix4fv( glGetUniformLocation(idProgram, "matModel"), 1, GL_FALSE, glm::value_ptr(matModel));

		// Rendering
		glBindVertexArray( idVAO );
		glDrawArrays( GL_TRIANGLES, 0, OBJ_vertices.size() );
		glBindVertexArray( 0 );


	// Wylaczanie
	glUseProgram( 0 );


	glutSwapBuffers();

}

// ---------------------------------------------------
void Initialize()
{
	// Ustawienia globalne
	glEnable(GL_DEPTH_TEST);
	glClearColor( 0.3f, 0.3f, 0.3f, 1.0f );

	// ------------------------------------------------------------
	// Ustawienie domyslnego odsuniecia kamery od polozenia (0,0,0)
	// ------------------------------------------------------------
	CameraTranslate_x = 0.0;
	CameraTranslate_y = -1.0;
	CameraTranslate_z = -12.0;


	// Wczytanie pliku obj
	if (!loadOBJ("scene.obj", OBJ_vertices, OBJ_uvs, OBJ_normals))
	{
		printf("Not loaded!\n");
		exit(1);
	}

	// Potok
	idProgram = glCreateProgram();
	glAttachShader( idProgram, LoadShader(GL_VERTEX_SHADER, "vertex.glsl"));
	glAttachShader( idProgram, LoadShader(GL_FRAGMENT_SHADER, "fragment.glsl"));
	LinkAndValidateProgram( idProgram );


	// VAO
	glGenVertexArrays( 1, &idVAO );
	glBindVertexArray( idVAO );
		// Bufor na wspolrzedne wierzcholkow
		GLuint idVBO_coord;
		glGenBuffers( 1, &idVBO_coord );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_coord );
		glBufferData( GL_ARRAY_BUFFER, OBJ_vertices.size() * sizeof(glm::vec3), &OBJ_vertices[0], GL_STATIC_DRAW );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 0 );
		// Bufor na wspolrzedne UV
		GLuint idVBO_uv;
		glGenBuffers( 1, &idVBO_uv );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_uv );
		glBufferData( GL_ARRAY_BUFFER, OBJ_uvs.size() * sizeof(glm::vec2), &OBJ_uvs[0], GL_STATIC_DRAW );
		glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 1 );
		// -----------------------------------------------
		// NOWE : Bufor na wektory normalne
		// -----------------------------------------------
		GLuint idVBO_normal;
		glGenBuffers( 1, &idVBO_normal );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_normal );
		glBufferData( GL_ARRAY_BUFFER, OBJ_normals.size() * sizeof(glm::vec3), &OBJ_normals[0], GL_STATIC_DRAW );
		glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 2 );
	glBindVertexArray( 0 );


}

// ---------------------------------------------------
void Reshape( int width, int height )
{
	glViewport( 0, 0, width, height );
	matProj = glm::perspective(glm::radians(80.0f), width/(float)height, 0.1f, 50.0f);
}

// ---------------------------------------------------
void Keyboard( unsigned char key, int x, int y )
{
    switch(key)
    {
		case 27:	// ESC key
			glutLeaveMainLoop();
			break;

    }
}



// ---------------------------------------------------
int main( int argc, char *argv[] )
{
	// GLUT
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
	glutInitContextVersion( 3, 3 );
	glutInitContextProfile( GLUT_CORE_PROFILE );
	glutInitWindowSize( 600, 500 );
	glutCreateWindow( "Programownie grafiki 3D w OpenGL" );

	glutDisplayFunc( DisplayScene );
	glutReshapeFunc( Reshape );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutMouseWheelFunc( MouseWheel );
	glutKeyboardFunc( Keyboard );
	glutSpecialFunc( SpecialKeys );



	// GLEW
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if( GLEW_OK != err )
	{
		printf("GLEW Error\n");
		exit(1);
	}

	// OpenGL
	if( !GLEW_VERSION_3_3 )
	{
		printf("Brak OpenGL 3.3!\n");
		exit(1);
	}


	Initialize();

	glutMainLoop();

	// Kod wykonywany po wyjsciu z glownej petli

	// Cleaning
	glDeleteProgram( idProgram );
	glDeleteVertexArrays( 1, &idVAO );


	return 0;
}
