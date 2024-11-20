#pragma once

#include <glad/gl.h>
#include "../Textures/Textures.h"
#include "../Shaders/Shaders.h"
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>

class Crosshair {
public:
	Crosshair();
	~Crosshair();

	void render(Shader* shader);

private:
	GLuint VAO, VBO, EBO;
};
