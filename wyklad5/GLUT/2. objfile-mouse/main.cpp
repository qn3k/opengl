// -------------------------------------------------
// Programowanie grafiki 3D w OpenGL / UG
// -------------------------------------------------
// Przyklad obslugi myszy we FreeGlut do obslugi
// polozenia i orientacji kamery (macierzy matView)
// -------------------------------------------------
#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
// Biblioteki GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// ----------------------------------------------------
// NOWE: Dodatkowe funkcje do obslugi myszy
// ----------------------------------------------------
#include "utilities.hpp"

// Wczytywanie plikow obj
#include "objloader.hpp"


// Macierze transformacji i rzutowania
glm::mat4 matProj;
glm::mat4 matView;
glm::mat4 matModel;

// Identyfikatory obiektow
GLuint idProgram;	// programu
GLuint idVAO;		// tablic wierzcholkow
GLuint idVBO_coord;	// bufora na wspolrzedne
GLuint idVBO_color; // bufora na kolory


// Wektory na dane z pliku OBJ
std::vector<glm::vec3> OBJ_vertices;
std::vector<glm::vec2> OBJ_uvs;
std::vector<glm::vec3> OBJ_normals;



// ---------------------------------------------------
void DisplayScene()
{
	// Czyszczenie bufora koloru i glebokosci
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );


	// ------------------------------------------------------------
	// NOWE: Uzywamy nowej funkcji UpdateViewMatrix z
	//       utilities.hpp, ktora wylicza macierz widoku
	// ------------------------------------------------------------
	matView = UpdateViewMatrix();



	// Wlaczenie programu
	glUseProgram( idProgram );


		// Przekazanie macierzy
		glUniformMatrix4fv( glGetUniformLocation(idProgram, "matProj"), 1, GL_FALSE, glm::value_ptr(matProj));
		glUniformMatrix4fv( glGetUniformLocation(idProgram, "matView"), 1, GL_FALSE, glm::value_ptr(matView));


		// Obiekt z pliku OBJ
		matModel = glm::mat4(1.0);
		glUniformMatrix4fv( glGetUniformLocation(idProgram, "matModel"), 1, GL_FALSE, glm::value_ptr(matModel));

		// rendering
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
	glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );

	// -----------------------------------------------
	// NOWE: Ustawienie domyslnego odsuniecia kamery
	// od polozenia (0,0,0)
	// -----------------------------------------------
	CameraTranslate_x = 0.0;
	CameraTranslate_y = -2.0;
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
		glGenBuffers( 1, &idVBO_coord );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_coord );
		glBufferData( GL_ARRAY_BUFFER, OBJ_vertices.size() * sizeof(glm::vec3), &OBJ_vertices[0], GL_STATIC_DRAW );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 0 );
		// Bufor na kolory
		glGenBuffers( 1, &idVBO_color );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_color );
		glBufferData( GL_ARRAY_BUFFER, OBJ_normals.size() * sizeof(glm::vec3), &OBJ_normals[0], GL_STATIC_DRAW );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 1 );
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
// NOWE: Funkcja zwrotna obslugujaca wybranie pozycji
// menu
// ---------------------------------------------------
void Menu( int value )
{
    switch( value )
    {
	case 1:
		printf("Wybrano opcja nr jeden\n")	;
		break;

	case 2:
		printf("Wybrano opcje nr dwa\n")	;
		break;

	default:
		printf("Wybrano %d \n", value);
    }
}

// ---------------------------------------------------
// NOWE: Funkcja tworzaca strukture menu GLUTA
// ---------------------------------------------------
void CreateGLUTMenu()
{
	// Najpierw tworzymy podmenu
	int podmenuA = glutCreateMenu( Menu );
	glutAddMenuEntry( "Pozycja 1", 1 );
	glutAddMenuEntry( "Pozycja 2", 2 );

	// Kolejne podmenu
	int podmenuB = glutCreateMenu( Menu );
	glutAddMenuEntry( "Pozycja 3", 3 );
	glutAddMenuEntry( "Pozycja 4", 4 );

	// Na koniec utworzenie glownego menu
	// wraz z ustawieniem funkcja zwrotna
	glutCreateMenu( Menu );
	glutAddMenuEntry( "Pozycja 5", 5 );
	glutAddSubMenu( "Podmenu A", podmenuA );
	glutAddSubMenu( "Podmenu B", podmenuB );

	// ---------------------------------------------------
	// Ustawienie
	glutAttachMenu( GLUT_RIGHT_BUTTON );
	// GLUT_LEFT_BUTTON, GLUT_MIDDLE_BUTTON, GLUT_RIGHT_BUTTON.
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
	glutCreateWindow( "Programownie grafiki w OpenGL" );

	// -----------------------------------
	// NOWE: ustawienie kilku dodatkowych
	// funkcji zwrotnych, w szczegolnosci
	// do obslugi myszy
	// -----------------------------------
	glutDisplayFunc( DisplayScene );
	glutReshapeFunc( Reshape );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutMouseWheelFunc( MouseWheel );
	glutKeyboardFunc( Keyboard );
	glutSpecialFunc( SpecialKeys );


	CreateGLUTMenu();


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
	glDeleteVertexArrays( 1, &idVBO_coord );
	glDeleteVertexArrays( 1, &idVBO_color );
	glDeleteVertexArrays( 1, &idVAO );


	return 0;
}
