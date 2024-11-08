#pragma once

#include <glad/gl.h>
#include "../Textures/Textures.h"

class Crosshair {
public:
	Crosshair();
	~Crosshair();

	void Render(GLuint shader);

private:
	GLuint VAO, VBO, EBO;
};
