#pragma once
#include <vector>
#include <iostream>

// ----------------------------------------------
// Przyklad nowej klasy do obslugi obiektow
// ktore dodatkowo udostepniaja obsluge testu
// kolizji
// ----------------------------------------------
// UWAGA! To tylko propozycja
// ----------------------------------------------

class CSceneObject
{
public:

	CSceneObject() = default;

	// Konstruktor
	void Init(CMesh* mesh)
	{
		this->Mesh = mesh;
	}

	// Glowna metoda renderujaca
	virtual void Draw() const
	{
		Mesh->Draw();
	}

	// Aktualizacja
	void Update()
	{
		// TO_DO: Wykorzystaj klase CGround i zaktualizuj
		// wspolrzedna Y obiektu zgodnie z podlozem

		// ....

		// nastepnie wyliczamy nowa macierz modelu i macierz normalna
		glm::mat4 newMatModel = glm::translate(glm::mat4(1.0), Position);
		newMatModel = glm::rotate(newMatModel, Angle, glm::vec3(0.0, 1.0, 0.0));

		Mesh->setMatModel(newMatModel);
		// TO_DO: nie zapomnij o macierzy normalnej
	}

	// ------------------------------------------
	// NOWE: Ustawienie pozycji obiektu na scenie
	// ------------------------------------------
	void setPosition(glm::vec3 _pos) {
		this->Position = _pos;
		this->Update();
	}

	// Inne settery i gettery
	const glm::vec3&	getPosition() {	return Position; }
	void 		setCollider(CCollider *coll) { this->Collider = coll; }
	CCollider * getCollider() const { return this->Collider; }
	float 		getAngle() const { return Angle; }
	void 		setAngle(float newAngle) { Angle = newAngle; this->Update(); }


private:

	// Wskaznik do obiektu mesha
	CMesh * Mesh = nullptr;

	// ------------------------------------------
	// NOWE: Collider do testu kolizji
	// ------------------------------------------
	CCollider* Collider = nullptr;

	// Pozycja i orientacja obiektu
	glm::vec3 Position = glm::vec3(0.0f);
	float Angle = 0.0f;

};

