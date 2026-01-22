#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>

#define __ALTITUDE_ERROR	99999

// -------------------------------
// Klasa reprezentujaca trojkat
class CTriangle
{
public:
	// Konstruktory
	CTriangle() = default;
	CTriangle(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3)
		: p{ p1, p2, p3 }
	{
		calculateEquation();
	}

	// Metody publiczne

	// Czy punkt jest nad trojkatem rzutujac go na plaszczyzne xy?
	[[nodiscard]] inline bool isAbove (const glm::vec2& point) const
	{
		float d1 = det(point, p[0], p[1]);
		float d2 = det(point, p[1], p[2]);
		float d3 = det(point, p[2], p[0]);

		// czy jeden z wyznacznikow ma znak ujemny
		bool isNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
		// czy jeden z wyznacznikow ma znak dodatni
		bool isPos = (d1 > 0) || (d2 > 0) || (d3 > 0);

		// jesli mamy dodatnie i ujemne wartosci wyznacznikow
		// to punkt nie lezy wewnatrz trojkata
		if (isNeg && isPos)
			return false;
		else
			return true;
	}

	// Obliczamy wysokosc punktu w trojkacie z rownania plaszczyzny
	[[nodiscard]]  inline float calcY(const glm::vec2& point) const
	{
		if (B) return -(A*point.x + C*point.y + D)/B;
		else { return __ALTITUDE_ERROR; }
	}

private:

	// Funkcje pomocnicze
	// obliczanie wspolczynnikow rownania
	void calculateEquation(void)
	{
		glm::vec3 v1 = p[1] - p[0];
		glm::vec3 v2 = p[2] - p[0];

		glm::vec3 normal = glm::cross(v1, v2);

		A = normal.x;
		B = normal.y;
		C = normal.z;

		D = - (A*p[0].x + B*p[0].y + C*p[0].z);

		if (B == 0)
		{
			std::cerr << "Uwaga! Trojkat pionowy.\n";
		}
	}

	// czy punkt p jest po lewej stronie odcinka (A, B)
	// obliczanie wyznacznika
	[[nodiscard]] static float det (glm::vec2 p, glm::vec3 A, glm::vec3 B)
	{
		return (p.x - B.x) * (A.z - B.z) - (A.x - B.x) * (p.y - B.z);
	}


	// Atrybuty
	glm::vec3 p[3];		// wspolrzedne punktow trojkata
	float A, B, C, D;	// wspolczynniki rownania plaszczyzny Ax + By + Cz + D = 0

};


// ---------------------------------------
// Klasa reprezentujaca podloze
// ---------------------------------------
class CGround
{
public:


	// stworzenie struktury z listy trojkatow
	// na przyklad przy okazji ladowania z pliku OBJ
	void Init(const std::vector<glm::vec3>& verts)
    {
        if (verts.size() % 3 != 0)
        {
            std::cerr << "B��d: liczba wierzcho�k�w nie jest wielokrotno�ci� 3!\n";
            return;
        }

        triangles.clear();
        triangles.reserve(verts.size() / 3);

        for (std::size_t i = 0; i < verts.size(); i += 3)
        {
            triangles.emplace_back(verts[i], verts[i + 1], verts[i + 2]);
        }

        std::cout << "Utworzono CGround zawierajacy "
                  << triangles.size() << " trojkatow.\n";
    }

	// ----------------------------------------
	// Glowna funkcja obliczajaca wysokosci nad podlozem w punkcie X Z
	// - przeszukanie po wszystkich trojkatach
	// - gdy zostanie znaleziony trojkat, obliczana jest wysokosc Y
	[[nodiscard]] float getY(const glm::vec2& point) const
	{

		// obliczenie listy trojkatow nad ktorymi jestesmy
		const CTriangle *collTriangle = NULL;

		// naiwna petla petla po wszystkich trojkatach
		for (const auto& tri : triangles)
        {
            if (tri.isAbove(point))
            {
                return tri.calcY(point);
            }
        }

        // nie znaleziono trojkata pod nogami
		std::cerr << "Brak ziemi pod nogami!\n";
        return 0.0f;
	}

private:

	// Atrybuty
	std::vector<CTriangle> triangles;	// wektor trojkatow siatki podloza

};
