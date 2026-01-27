#pragma once
#include <iostream>

// ----------------------------------------------
// Klasa do reprezentacji postaci
// - obiektu, ktory porusza sie po scenie
// ----------------------------------------------

// ----------------------------------------------
// Klasa do reprezentacji postaci
// - obiektu, ktory porusza sie po scenie
// ----------------------------------------------
class CPlayer : public CSceneObject
{
public:

	// zmiana polozenia na scenie bez testu kolizji
	void Move(float speed)
	{
		setPosition(getPosition() + Direction * speed);
		CSceneObject::Update();
	}

	// ----------------------------------------------
	// NOWE: zmiana polozenia uwzgledniajaca kolizje
	// ----------------------------------------------
	void Move(float speed, const CSceneObject &_other)
	{
		// Kopia polozenia
		glm::vec3 oldPosition = getPosition();

		// aktualizujemy polozenie
		CSceneObject::setPosition(getPosition() + Direction * speed);

		// sprawdzamy kolizje
		if (getCollider()->isCollision(_other.getCollider()))
		{
			std::cout << "Mamy kolizje!\n";
			CSceneObject::setPosition(oldPosition);
			return;
		}
		Update();	// aktualizacja
	}


	// zmiana orientacji obiektu
	void Rotate(float angle)
	{
		float newAngle = CSceneObject::getAngle() + angle;
		CSceneObject::setAngle(newAngle);

		// wyliczanie nowego wektora kierunku
		Direction.x = cos(newAngle);
		Direction.z = -sin(newAngle);
	}
private:

	// wektor ruchu (UWAGA! wyliczany z Angle)
	glm::vec3 Direction = glm::vec3(1.0, 0.0, 0.0);



};
