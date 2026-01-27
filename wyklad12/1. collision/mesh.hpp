#pragma once
#include <vector>
#include <iostream>
#include <memory>

// ----------------------------------------------
// Przyklad implementacji klasy do obslugi
// obiektow 3D renderowanych w OpenGL
// ----------------------------------------------

class CMesh
{
public:

	CMesh() = default;

	// inicjalizacja (obecnie tylko to co jest
	// potrzebne do renderingu
	void Init(const char *_obj_file, const char *_tex_file)
	{

		// OBJ
		if (!loadOBJ(_obj_file, OBJ_vertices, OBJ_uvs, OBJ_normals))
		{
			std::cerr << "OBJ file " << _obj_file << " not loaded!\n";
			exit(1);
		}

		glGenVertexArrays( 1, &idVAO );
		glBindVertexArray( idVAO );
		glGenBuffers( 1, &vBuffer_pos );
		glBindBuffer( GL_ARRAY_BUFFER, vBuffer_pos );
		glBufferData( GL_ARRAY_BUFFER, OBJ_vertices.size() * sizeof(glm::vec3), &(OBJ_vertices)[0], GL_STATIC_DRAW );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 0 );
		glGenBuffers( 1, &vBuffer_uv );
		glBindBuffer( GL_ARRAY_BUFFER, vBuffer_uv );
		glBufferData( GL_ARRAY_BUFFER, OBJ_uvs.size() * sizeof(glm::vec2), &(OBJ_uvs)[0], GL_STATIC_DRAW );
		glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 1 );
		glBindVertexArray( 0 );

		// Tekstura
		int tex_width, tex_height, tex_n;
		unsigned char *tex_data;
		tex_data = stbi_load(_tex_file, &tex_width, &tex_height, &tex_n, 0);
		if (!tex_data)
		{
			std::cerr << "Texture error!\n";
			exit(1);
		}
		glGenTextures(1, &idTexture);
		glBindTexture(GL_TEXTURE_2D, idTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}

	// Czyszczenie
	void Clean()
	{
		glDeleteVertexArrays(1, &idVAO);
		glDeleteBuffers(1, &vBuffer_pos);
		glDeleteBuffers(1, &vBuffer_uv);

	}

	// rendering na scenie
	void Draw()
	{
		// pobieranie aktualnego programu
		GLint idProgram;
		glGetIntegerv(GL_CURRENT_PROGRAM, &idProgram);

		// wyslanie macierzy modelu
		glUniformMatrix4fv( glGetUniformLocation( idProgram, "matModel" ), 1, GL_FALSE, glm::value_ptr(matModel) );

		// aktywacja tekstury
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(idProgram, "tex0"), 0);
		glBindTexture(GL_TEXTURE_2D, idTexture);

		// rendering
		glBindVertexArray( idVAO );
		glDrawArrays( GL_TRIANGLES, 0, OBJ_vertices.size() );
		glBindVertexArray( 0 );
	}

	// ustawianie macierzy modelu
	void setMatModel(const glm::mat4& mat)
	{
		this->matModel = mat;
	}

private:

	// potok openGL
	GLuint idVAO, vBuffer_pos, vBuffer_uv;
	GLuint idTexture;

	std::vector<glm::vec3> OBJ_vertices;
	std::vector<glm::vec3> OBJ_normals;
	std::vector<glm::vec2> OBJ_uvs;


	// macierz modelu
	glm::mat4x4 matModel = glm::mat4(1.0);
	// TODO: brakuje macierzy normalnej

};
