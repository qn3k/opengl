// -----------------------------------------------
// Przykladowy szablon budowy klasy CCamera oraz
// pochodnych do obslugi kamery
// UWAGA: nalezy uzupelnic m.in. TO_DO
// -----------------------------------------------
#pragma once
#define PI	3.1415926535

// -----------------------------------------------
class CCamera
{

public:

	// Domyslny konstruktor i domyslne ustawienia
	// ktore mozna swobodnie zmienic
	CCamera()
	{

		// Domyslne wartosci skladowych
		// (do zmiany wg potrzeby)
		Up = glm::vec3(0.0f, 1.0f, 0.0f);
		Position = glm::vec3(0.0f, 4.0f, -5.f);
		Angles = glm::vec3(-0.5f, -1.34f, 0.0f);
		Fov = 80.0f; // w stopniach

		NearPlane = 0.1f;
		FarPlane = 150.0f;

		// Wywolanie metody aktualizujacej
		// m.in. Direction i matView
		this->Update();
	}

	// Wirtualna metoda aktualizujaca dane kamery
	// przydatna w klasach pochodnych
	virtual void Update()
	{
		// wektor Direction
		Direction.x = cos(Angles.y) * cos(Angles.x);
		Direction.y = sin(Angles.x);
		Direction.z = -sin(Angles.y) * cos(Angles.x);

		// macierz widoku
		matView = glm::lookAt(Position, Position+Direction, Up);

		//printf("camera update (%f,%f,%f) (%f,%f,%f)!\n", Position.x, Position.y, Position.z, Angles.x, Angles.y, Angles.z);
	}

	// Metoda aktualizujaca macierz projection
	// wywolywana np. w Reshape()
	void UpdatePerspective(float width, float height)
	{
		Width = width;
		Height = height;
		matProj = glm::perspectiveFov(glm::radians(Fov), Width, Height, NearPlane, FarPlane);
	}

	// Metoda aktualizujaca macierz projection
	// wywolywana np. w Reshape()
	void UpdateOrtho(float width, float height)
	{
		// TO_DO:
		// matProj = glm::ortho(...);
	}

	// przesylanie obu macierzy do programu pod
	// wskazane nazwy zmiennych uniform
	void SetUniformPV(GLuint idProgram, const char *proj = "matProj", const char *view = "matView")
	{
		glUniformMatrix4fv( glGetUniformLocation( idProgram, proj ), 1, GL_FALSE, glm::value_ptr(matProj) );
		glUniformMatrix4fv( glGetUniformLocation( idProgram, view ), 1, GL_FALSE, glm::value_ptr(matView) );
	}

	// przesylanie iloczynu macierzy matProj*matView
	// do programu pod wskazana nazwe
	void SetUniformPV(GLuint idProgram, const char *projview = "matPV")
	{
		glm::mat4 matProjView = matProj * matView;
		glUniformMatrix4fv( glGetUniformLocation( idProgram, projview ), 1, GL_FALSE, glm::value_ptr(matProjView) );
	}

	// zmiana kata widzenia kamery (np. przy zoomowaniu)
	void AddFov(GLfloat _fov)
	{
		this->Fov += _fov;
		this->UpdatePerspective(this->Width, this->Height);
	}

protected:

	// Macierze rzutowania i widoku
	glm::mat4 matProj;
	glm::mat4 matView;

	// Atrybuty kontrolujace matView
	glm::vec3 Position;        // polozenie kamery
	glm::vec3 Angles;          // x = pitch, y = yaw, z = roll (nieuzywane)
	glm::vec3 Up;              // domyslnie wektor (0,1,0)
	glm::vec3 Direction;       // wektor kierunku obliczany z Angles

	// Atrybuty kontrolujace matProj
	float Width, Height;       // proporcje bryly obcinania
	float NearPlane, FarPlane; // plaszczyzny tnace
	float Fov;	               // kat widzenia kamery

};


// ----------------------------------------------------------
class CFreeCamera : public CCamera
{

public:

	void Init(int windowWidth, int windowHeight)
	{
		// TO_DO: uzupelnij wedle uznania

		// ustawienie macierzy projection pierwszy raz
		// Obliczanie matProj po raz pierwszy
		CCamera::UpdatePerspective(windowWidth, windowHeight);
	}

	// Zmiana polozenia korzystajaca z wektora Direction
	void Move(float val)
	{
		Position += Direction*val;

		// Aktualizacja danych kamery
		CCamera::Update();
	}

	// Zmiana orientacji (wektora Direction) ktory
	// wyliczany jest z katow w Angles
	void Rotate(float pitch, float yaw)
	{
		Angles.x += pitch;
		Angles.y += yaw;

		// Zawijanie
		if (Angles.x > PI/2.0)  Angles.x =  PI/2.0;
		if (Angles.x < -PI/2.0) Angles.x = -PI/2.0;
		if (Angles.y > 2.0*PI)  Angles.y -= 2.0*PI;
		if (Angles.y < -2.0*PI) Angles.y += 2.0*PI;

		// Aktualizacja danych kamery
		CCamera::Update();
	}

	// TODO: warto dopisac metode poruszania kamera
	// na boki (strafing)

};


// ----------------------------------------------------------
class CFPSCamera : public CCamera
{

public:

	// Dobrym pomyslem jest podlaczenie kamery pod obiekt
	// postaci, ktora bedzie kontrolowala polozenie i
	// orientacje kamery

	// CPlayer *Player = nullptr;

	// Opcjonalnie mozemy zdefiniowac wektor przesuniecia
	// kamery wzgledem obiektu, z ktorym jestesmy podlaczeni
	// (polozenie oczu obiektu = polozenie kamery)

	// glm::vec3 ShiftUp;


	void Init()
	{
		// TO_DO: uzupelnij wedle uznania
		// Player =
		// ShiftUP =
	}


	// aktualizacja danych kamery na podstawie
	// obiektu klasy CPlayer (lub innego)
	// UWAGA: ta metoda powinna byc aktualizowana
	// co najmniej zawsze wtedy gdy obiekt CPlayer
	// zmienia swoje polozenie/orientacje
	virtual void Update()
	{
		/*
		// Ustawienie katow orientacji kamery
		// zgodnie z katami postaci
		Angles = glm::vec3(0.0, Player->Angle, 0.0);

		// Ustawienie polozenia kamery zgodnie
		// z polozeniem postaci
		Position = Player->Position + ShiftUp;

		CCamera::Update();
		*/
	}
};


// -------------------------------------------------------
// Tutaj mozemy wykorzystac klase CFreeCamera lub
// napisac niezaleznie i dziedziczyc po CCamera
// -------------------------------------------------------
class CTPSCamera : public CFPSCamera
{

public:
	// ---------------------------------------------
	void Init()
	{
		// TO_DO: uzupelnij wedle uznania
	}

	// ---------------------------------------------
	void Update()
	{

		// TO_DO: uzupelnij wedle uznania
	}

private:

	// dodatkowe przesuniecie kamery trzecioosobowej
	// w przypadku kamery TPS bedzie ono wyliczane
	// z wektora Direction
	glm::vec3 ShiftBack;

};

