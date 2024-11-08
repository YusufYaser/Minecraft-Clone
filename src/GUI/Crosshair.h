#pragma once

#include <glad/gl.h>
#include "../Textures/Textures.h"
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>

class Crosshair {
public:
	Crosshair();
	~Crosshair();

	void Render(GLuint shader);

private:
	GLuint VAO, VBO, EBO;
};
