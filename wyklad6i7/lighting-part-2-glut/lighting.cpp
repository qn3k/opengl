// -------------------------------------------------
// Programowanie grafiki 3D w OpenGL / UG
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
#include "obj_loader.hpp"

using glm::vec3;

// Obiekty na scenie
enum {
	OBJ_GROUND,	// ziemia
	OBJ_SUZANNE	// obiekt na scenie
};

// Macierze PVM
glm::mat4 matProj;
glm::mat4 matView;
glm::mat4 matModel[2];


// Identyfikatory obiektow
GLuint idProgram;
GLuint idVAO[2];

// Wektory na dane z pliku OBJ
std::vector<glm::vec3> OBJ_vertices[2];
std::vector<glm::vec2> OBJ_uvs[2];
std::vector<glm::vec3> OBJ_normals[2];


// Struktura parametrow swiatla
struct LightParam
{
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
	vec3 Attenuation;
	vec3 Position; // lub Direction dla kierunkowego
};

// Struktura materialu obiektu
struct MaterialParam
{
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
	float Shininess;
};


// Przykladowe swiatlo punktowe
struct LightParam myLight =
{
	vec3 (0.1 , 0.1 , 0.1), // ambient
	vec3 (1.0 , 1.0 , 1.0), // diffuse
	vec3 (1.0 , 1.0 , 1.0), // specular
	vec3 (1.0 , 0.0 , 0.01), // attenuation
	vec3 (2.0 , 3.0 , 1.0)  // position
};


// Przykladowe materialy
struct MaterialParam myMaterial[2] =
{
	{
		vec3 (0.2, 0.2, 0.2), // ambient
		vec3 (1.0, 1.0, 1.0), // diffuse
		vec3 (0.5, 0.5, 0.5), // specular
		32.0 // shininess
	},
	{
		vec3 (0.2, 0.2, 0.2), // ambient
		vec3 (1.0, 1.0, 1.0), // diffuse
		vec3 (1.0, 1.0, 1.0), // specular
		8.0 // shininess
	}
};


float Time = 0.0;

// ---------------------------------------------------
// Przeslanie parametrow oswietlenia do shadera
void sendLightParameters(LightParam light){
	// pobranie id aktualnego programu
    GLint programId;
    glGetIntegerv(GL_CURRENT_PROGRAM, &programId);

    glUniform3fv(glGetUniformLocation(programId, "myLight.Ambient"), 1, glm::value_ptr(light.Ambient));
    glUniform3fv(glGetUniformLocation(programId, "myLight.Diffuse"), 1, glm::value_ptr(light.Diffuse));
    glUniform3fv(glGetUniformLocation(programId, "myLight.Specular"), 1, glm::value_ptr(light.Specular));
    glUniform3fv(glGetUniformLocation(programId, "myLight.Attenuation"), 1, glm::value_ptr(light.Attenuation));
    glUniform3fv(glGetUniformLocation(programId, "myLight.Position"), 1, glm::value_ptr(light.Position));
}

// ---------------------------------------------------
// Przeslanie parametrow materialow do shadera
void sendMaterialParameters(MaterialParam material){
	// pobranie id aktualnego programu
    GLint programId;
    glGetIntegerv(GL_CURRENT_PROGRAM, &programId);

    glUniform3fv(glGetUniformLocation(programId, "myMaterial.Ambient"), 1, glm::value_ptr(material.Ambient));
    glUniform3fv(glGetUniformLocation(programId, "myMaterial.Diffuse"), 1, glm::value_ptr(material.Diffuse));
    glUniform3fv(glGetUniformLocation(programId, "myMaterial.Specular"), 1, glm::value_ptr(material.Specular));
    glUniform1f(glGetUniformLocation(programId, "myMaterial.Shininess"), material.Shininess);
}

// ---------------------------------------------------
void DisplayScene()
{
	// Czyszczenie bufora koloru i glebokosci
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// Jesli ponizsze powoduje blad kompilacji - zakomentuj
	__CHECK_FOR_ERRORS

	// Wyliczanie macierz widoku
	matView = UpdateViewMatrix();

	// Wyliczanie pozycji kamery z matView
	glm::vec3 cameraPos = ExtractCameraPos(matView);


	// Wlaczenie programu
	glUseProgram( idProgram );


		// Przekazanie macierzy rzutowania i pozycje kamery
		glUniformMatrix4fv( glGetUniformLocation(idProgram, "matProj"), 1, GL_FALSE, glm::value_ptr(matProj));
		glUniformMatrix4fv( glGetUniformLocation(idProgram, "matView"), 1, GL_FALSE, glm::value_ptr(matView));
		glUniform3fv( glGetUniformLocation( idProgram, "cameraPos" ), 1, &cameraPos[0] );

		// Przekazanie parametrow oswietlenia
        sendLightParameters(myLight);

		// RENDERING: Podloze
		{
			// Material
			sendMaterialParameters(myMaterial[OBJ_GROUND]);
			// Macierz modelu obiektu
			matModel[OBJ_GROUND] = glm::mat4(1.0);
			glUniformMatrix4fv( glGetUniformLocation(idProgram, "matModel"), 1, GL_FALSE, glm::value_ptr(matModel[OBJ_GROUND]));

			// Rendering
			glBindVertexArray( idVAO[OBJ_GROUND] );
			glDrawArrays( GL_TRIANGLES, 0, OBJ_vertices[OBJ_GROUND].size() );
			glBindVertexArray( 0 );
		}

		// RENDERING: Suzanne
		{
			// Material
			sendMaterialParameters(myMaterial[OBJ_SUZANNE]);
			// Macierz modelu obiektu
			matModel[OBJ_SUZANNE] = glm::mat4(1.0);
			// TO_DO: Animacja Suzanne
			glUniformMatrix4fv( glGetUniformLocation(idProgram, "matModel"), 1, GL_FALSE, glm::value_ptr(matModel[OBJ_SUZANNE]));

			// Rendering
			glBindVertexArray( idVAO[OBJ_SUZANNE] );
			glDrawArrays( GL_TRIANGLES, 0, OBJ_vertices[OBJ_SUZANNE].size() );
			glBindVertexArray( 0 );
		}



	glUseProgram( 0 );
	glutSwapBuffers();
}

// ---------------------------------------------------
void Initialize()
{
	// Ustawienia globalne
	glEnable(GL_DEPTH_TEST);
	glClearColor( 0.3f, 0.3f, 0.3f, 1.0f );

	// Ustawienie domyslnego odsuniecia kamery od polozenia (0,0,0)
	CameraTranslate_x = 0.0;
	CameraTranslate_y = -1.0;
	CameraTranslate_z = -12.0;

	// Potok
	idProgram = glCreateProgram();
	glAttachShader( idProgram, LoadShader(GL_VERTEX_SHADER, "vertex.glsl"));
	glAttachShader( idProgram, LoadShader(GL_FRAGMENT_SHADER, "fragment.glsl"));
	LinkAndValidateProgram( idProgram );

	// Wczytanie pliku obj
	if (!loadOBJ("scene.obj", OBJ_vertices[OBJ_GROUND], OBJ_uvs[OBJ_GROUND], OBJ_normals[OBJ_GROUND]))
	{
		printf("Not loaded!\n");
		exit(1);
	}

	GLuint idVBO_coord, idVBO_uv, idVBO_normal;

	// VAO
	glGenVertexArrays( 1, &idVAO[OBJ_GROUND] );
	glBindVertexArray( idVAO[OBJ_GROUND] );
		// Bufor na wspolrzedne wierzcholkow
		glGenBuffers( 1, &idVBO_coord );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_coord );
		glBufferData( GL_ARRAY_BUFFER, OBJ_vertices[OBJ_GROUND].size() * sizeof(glm::vec3), &OBJ_vertices[OBJ_GROUND][0], GL_STATIC_DRAW );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 0 );
		// Bufor na wspolrzedne UV
		glGenBuffers( 1, &idVBO_uv );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_uv );
		glBufferData( GL_ARRAY_BUFFER, OBJ_uvs[OBJ_GROUND].size() * sizeof(glm::vec2), &OBJ_uvs[OBJ_GROUND][0], GL_STATIC_DRAW );
		glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 1 );
		// Bufor na wektory normalne
		glGenBuffers( 1, &idVBO_normal );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_normal );
		glBufferData( GL_ARRAY_BUFFER, OBJ_normals[OBJ_GROUND].size() * sizeof(glm::vec3), &OBJ_normals[OBJ_GROUND][0], GL_STATIC_DRAW );
		glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 2 );
	glBindVertexArray( 0 );

	// Wczytanie pliku obj
	if (!loadOBJ("suzanne.obj", OBJ_vertices[OBJ_SUZANNE], OBJ_uvs[OBJ_SUZANNE], OBJ_normals[OBJ_SUZANNE]))
	{
		printf("Not loaded!\n");
		exit(1);
	}

	// VAO
	glGenVertexArrays( 1, &idVAO[OBJ_SUZANNE] );
	glBindVertexArray( idVAO[OBJ_SUZANNE] );
		// Bufor na wspolrzedne wierzcholkow
		glGenBuffers( 1, &idVBO_coord );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_coord );
		glBufferData( GL_ARRAY_BUFFER, OBJ_vertices[OBJ_SUZANNE].size() * sizeof(glm::vec3), &OBJ_vertices[OBJ_SUZANNE][0], GL_STATIC_DRAW );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 0 );
		// Bufor na wspolrzedne UV
		glGenBuffers( 1, &idVBO_uv );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_uv );
		glBufferData( GL_ARRAY_BUFFER, OBJ_uvs[OBJ_SUZANNE].size() * sizeof(glm::vec2), &OBJ_uvs[OBJ_SUZANNE][0], GL_STATIC_DRAW );
		glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 1 );
		// Bufor na wektory normalne
		glGenBuffers( 1, &idVBO_normal );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_normal );
		glBufferData( GL_ARRAY_BUFFER, OBJ_normals[OBJ_SUZANNE].size() * sizeof(glm::vec3), &OBJ_normals[OBJ_SUZANNE][0], GL_STATIC_DRAW );
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
void Animation(int frame)
{
	Time += 0.01;

	glutPostRedisplay();
	glutTimerFunc(1000/60, Animation, 0);
}

// ---------------------------------------------------
int main( int argc, char *argv[] )
{
	// GLUT
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
	glutInitContextVersion( 3, 2 );
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
	if( !GLEW_VERSION_3_2 )
	{
		printf("Brak OpenGL 3.2!\n");
		exit(1);
	}

	Initialize();

	glutTimerFunc(1000/60, Animation, 0);
	glutMainLoop();


	return 0;
}
