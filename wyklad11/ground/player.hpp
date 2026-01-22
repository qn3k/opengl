#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>

// ----------------------------------------------
// Klasa do reprezentacji postaci
// - obiektu, ktory porusza sie po scenie
// ----------------------------------------------
class CPlayer
{

public:

	// Domyslny konstruktor
	CPlayer() = default;

	// Inicjalizacja obiektu
	void Init(const CGround *ground)
	{
		// ustawiamy wskaznik do obiekty CGround
		// aby moc aktualizowac wysokosc na podlozu
		myGround = ground;

		// aktualizacja polozenia wzgledem podloza
		// aktualizacja macierzy modelu/normalnej itp.
		Update();


		// ----	------------------------------
		// TO_DO: Uzupelnij wedle uznania
		// Wykorzystaj swoja wlasna klase CMesh
		// w tym miejscu
		// ----------------------------------

		// Ludzik LEGO
		{
			// Tekstura
			int tex_width, tex_height, tex_n;
			unsigned char *tex_data;

			tex_data = stbi_load("assets/lego.png", &tex_width, &tex_height, &tex_n, 0);

			glGenTextures(1, &idTexture[LEGO]);
			glBindTexture(GL_TEXTURE_2D, idTexture[LEGO]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data);
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

			// VAO z pliku OBJ
			if (!loadOBJ("assets/LEGO.obj", OBJ_vertices[LEGO], OBJ_uvs[LEGO], OBJ_normals[LEGO]))
			{
				std::cerr << "OBJ not loaded!\n";
				exit(1);
			}

			glGenVertexArrays( 1, &idVAO[LEGO] );
			glBindVertexArray( idVAO[LEGO] );
			GLuint vBuffer_pos, vBuffer_uv;
			glGenBuffers( 1, &vBuffer_pos );
			glBindBuffer( GL_ARRAY_BUFFER, vBuffer_pos );
			glBufferData( GL_ARRAY_BUFFER, OBJ_vertices[LEGO].size() * sizeof(glm::vec3), &(OBJ_vertices[LEGO])[0], GL_STATIC_DRAW );
			glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
			glEnableVertexAttribArray( 0 );
			glGenBuffers( 1, &vBuffer_uv );
			glBindBuffer( GL_ARRAY_BUFFER, vBuffer_uv );
			glBufferData( GL_ARRAY_BUFFER, OBJ_uvs[LEGO].size() * sizeof(glm::vec2), &(OBJ_uvs[LEGO])[0], GL_STATIC_DRAW );
			glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, NULL );
			glEnableVertexAttribArray( 1 );
			glBindVertexArray( 0 );
		}
	}

	// ----	------------------------------
	// TO_DO: Uzupelnij wedle uznania
	// Wykorzystaj swoja wlasna klase CMesh
	// w tym miejscu
	// ----------------------------------
	// renderowanie obiektu
	void Draw()
	{
		// pobieranie aktualnego potoku
		GLint idProgram = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &idProgram);

		// TO_DO: a moze wykorzystac metode Draw() z klasy CMesh?
		glBindVertexArray( idVAO[LEGO] );
			glUniformMatrix4fv( glGetUniformLocation( idProgram, "matModel" ), 1, GL_FALSE, glm::value_ptr(this->matModel) );
			glBindTexture(GL_TEXTURE_2D, idTexture[LEGO]);
			glDrawArrays( GL_TRIANGLES, 0, OBJ_vertices[LEGO].size() );
		glBindVertexArray( 0 );
	}

	// Obliczenie wysokosci nad ziemia
	void Update()
	{
		// aktualizacja polozenia na Y wzgledem podloza
		if (myGround)
        {
			Position.y = myGround->getY(glm::vec2(Position.x, Position.z));
        }

		// wyliczamy nowa macierz modelu i macierz normalna
		matModel = glm::translate(glm::mat4(1.0), Position);
		matModel = glm::rotate(matModel, Angle, glm::vec3(0.0, 1.0, 0.0));
		matNormal = glm::mat3(glm::transpose(glm::inverse(matModel)));
	}


	// ustawienie pozycji na scenie
	void SetPosition(const glm::vec3 pos)
	{
		Position = pos;
		Update();	// aktualizacja
	}

	// zmiana pozycji na scenie
	void Move(float speed)
	{
		Position += Direction * speed;
		Update();	// aktualizacja
	}

	// zmiana orientacji obiektu
	void Rotate(float angle)
	{
		Angle += angle;
		// wyliczanie nowego wektora kierunku
		Direction.x = cos(Angle);
		Direction.z = -sin(Angle);

		Update();	// aktualizacja
	}

private:

	// Atrybuty
	glm::vec3 Position = glm::vec3(0.0f);	// pozycja obiektu
	float Angle = 0.0f;	// kat orientacji obiektu w ukladzie sceny

	// wektor orientacji (UWAGA! wyliczany z Angle)
	glm::vec3 Direction = glm::vec3(1.0f, 0.0f, 0.0f);

	// wskaznik do obiektu podloza
	const CGround *myGround = nullptr;


	// macierz modelu i normalna
	// TO_DO: zdecydowanie lepiej wykorzystac
	// swoja klase CMesh
	glm::mat4 matModel = glm::mat4(1.0f);
	glm::mat4 matNormal = glm::mat4(1.0f);


};
