// ---------------------------------------------
// Rzutowanie prostokatne
// ---------------------------------------------

// Parametry
float right = 1;
float left = -1;
float top = 1;
float bottom = -1;
float far = 1.0;
float near = -1.0;
	
// Macierz rzutowania prostokatnego
mat4 Ortho;
Ortho[0] = vec4(2.0/(right-left), 0, 0, 0);
Ortho[1] = vec4(0, 2.0/(top-bottom), 0, 0);
Ortho[2] = vec4(0, 0, -2.0/(far-near), 0);
Ortho[3] = vec4(-(right+left)/(right-left), -(top+bottom)/(top-bottom), -(far+near)/(far-near), 1);

// ---------------------------------------------
// Rzutowanie perspektywiczne (A)
// ---------------------------------------------

// Parametry
float right = 1;
float left = -1;
float top = 1;
float bottom = -1;
float far = 5;
float near = 1;
	
// Macierz rzutowania perspektywicznego
mat4 PerspA;
PerspA[0] = vec4(2.0*near/(right-left), 0, 0, 0);
PerspA[1] = vec4(0, 2.0*near/(top-bottom), 0, 0);
PerspA[2] = vec4((right+left)/(right-left), (top+bottom)/(top-bottom), -(far+near)/(far-near), -1);
PerspA[3] = vec4(0, 0, -2.0*far*near/(far-near), 0);


// ---------------------------------------------
// Rzutowanie perspektywiczne (B)
// ---------------------------------------------

// Parametry
float fovy = 3.14/3.0; // 60 stopni
float aspect = 3.0/2.0;
float near = 1;
float far = 5;
	
// Macierz rzutowania perspektywicznego
mat4 PerspB;
PerspB[0] = vec4(1/(tan(fovy/2.0)*aspect), 0, 0, 0);
PerspB[1] = vec4(0, 1/tan(fovy/2), 0, 0);
PerspB[2] = vec4(0, 0, -(far+near)/(far-near), -1);
PerspB[3] = vec4(0, 0, -2.0*far*near/(far-near), 0);
