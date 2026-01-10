// -------------------------------------------------
// Programowanie grafiki 3D w OpenGL / UG
// -------------------------------------------------
// Funkcja loadOBJ wczytujaca plik w formacie OBJ
// Nie jest idealna, nie wczytuje informacji
// o materialach. Do bardziej powaznych zastosowan
// uzyj np. Assimp'a.
// -------------------------------------------------
#ifndef __OBJLOADER_H
#define __OBJLOADER_H

#include <vector>
#include <stdio.h>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

#include <glm/glm.hpp>

// Naglowek funkcji
bool loadOBJ(const char * Filename, std::vector<glm::vec3> & out_coords, std::vector<glm::vec2> & out_uvs, std::vector<glm::vec3> & out_normals);


// Implementacja
bool loadOBJ(const char * Filename, std::vector<glm::vec3> & out_coords, std::vector<glm::vec2> & out_uvs, std::vector<glm::vec3> & out_normals)
{
	std::cout << "Loading OBJ file " << Filename << " ...";

	// Wektor na indeksy
	std::vector<unsigned int> indices_v, indices_uv, indices_vn;

	// Wektory na unikalne dane wierzcholkow
	std::vector<glm::vec3> uniq_vertices;
	std::vector<glm::vec2> uniq_uvs;
	std::vector<glm::vec3> uniq_normals;
	unsigned int numberTriangles = 0;

	std::ifstream file(Filename);
	if (!file.is_open()) {
		std::cerr << "Can't open the file." << std::endl;
		return false;
	}

	// Dodanie pustych wspolrzednych, uvlek i normalek
	// przydadza sie w przypadku braku ktoregos z nich
	uniq_vertices.push_back(glm::vec3(0.0, 0.0, 0.0));
	uniq_uvs.push_back(glm::vec2(0.0, 0.0));
	uniq_normals.push_back(glm::vec3(0.0, 0.0, 0.0));

	// Glowna petla wczytujaca linia po linii z pliku
	std::string file_line, type;
	while (std::getline(file, file_line))
	{
		if (file_line.empty()) continue;

		// Obiekt String stream do parsowania
		std::istringstream strLine(file_line);
		strLine >> type;


		// Wspolrzedne tekstur
		if (type == "vt")
		{
			float u, v;
			strLine >> u >> v;
			uniq_uvs.push_back(glm::vec2(u, v));
		}
		// Wektory normalne
		else if (type == "vn")
		{
			float x, y, z;
			strLine >> x >> y >> z;
			uniq_normals.push_back(glm::vec3(x, y, z));
		}
		// Wspolrzedne wierzcholka
		else if (type == "v")
		{
			float x, y, z;
			strLine >> x >> y >> z;
			uniq_vertices.push_back(glm::vec3(x, y, z));
		}
		// Sciany (trojkaty)
		else if (type == "f")
		{
			bool new_face = false;
			char slash = '/';
			int format_face = std::count(file_line.begin(), file_line.end(), slash);

			unsigned int idx_v[3];
			unsigned int idx_uv[3];
			unsigned int idx_vn[3];


			// Format v/uv/vn v/uv/vn v/uv/vn
			if (format_face == 6)
			{
				// If the string contains two slashes
				// so there is no UV
				// Format v//vn v//vn v//vn
				if (file_line.find("//") != std::string::npos)
				{
					for (int i=0; i<3; i++)
					{
						strLine >> idx_v[i];
						strLine.ignore(1);
						idx_uv[i] = 0;
						strLine.ignore(1);
						strLine >> idx_vn[i];
					}
				}
				else
				{
					for (int i=0; i<3; i++)
					{
						strLine >> idx_v[i];
						strLine.ignore(1);
						strLine >> idx_uv[i];
						strLine.ignore(1);
						strLine >> idx_vn[i];
					}
				}

				new_face = true;

			}
			else
			// Format v/uv v/uv v/uv
			if (format_face == 3)
			{
				for (int i=0; i<3; i++)
				{
					strLine >> idx_v[i];
					strLine.ignore(1);
					strLine >> idx_uv[i];
					idx_vn[i] = 0;
				}

				new_face = true;
			}
			else
			// Format v v v
			if (format_face == 0)
			{
				for (int i=0; i<3; i++)
				{
					strLine >> idx_v[i];
					idx_uv[i] = 0;
					idx_vn[i] = 0;
				}

				new_face = true;
			}
			else
			{
				printf("Not supported format! Did you triangulate the object?\n");
				return false;
			}

			// Dodajemy indeksy calego trojkata
			if (new_face)
			{
				indices_v.push_back(idx_v[0]);
				indices_v.push_back(idx_v[1]);
				indices_v.push_back(idx_v[2]);
				indices_uv.push_back(idx_uv[0]);
				indices_uv.push_back(idx_uv[1]);
				indices_uv.push_back(idx_uv[2]);
				indices_vn.push_back(idx_vn[0]);
				indices_vn.push_back(idx_vn[1]);
				indices_vn.push_back(idx_vn[2]);

				numberTriangles++;
			}

		}
		else
		{
			// Pomijamy inne linie pliku OBJ
		}

	}

	// Na koniec eksportujemy wektor wspolrzednych
	// uvalek oraz normalki
	for (int i=0; i<numberTriangles*3; i++)
	{
		// Utworzenie danych wspolrzednych, uvlek i normalek
		// z odpowiednich wektorow na podstawie indeksow
		// pobranych w linii z prefiksem f
		glm::vec3 vertex = uniq_vertices[ indices_v[i] ];
		glm::vec2 uv = uniq_uvs[ indices_uv[i] ];
		glm::vec3 normal = uniq_normals[ indices_vn[i] ];

		// Eksport
		out_coords.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);
	}


	file.close();
	std::cout << " done.\n";
	return true;
}

#endif
