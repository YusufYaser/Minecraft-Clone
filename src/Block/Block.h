#pragma once
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/gl.h>

enum class BLOCK_TYPE {
	STONE = 1
};

class Block {
public:
	Block(BLOCK_TYPE type, glm::vec3 pos);
	~Block();

	void Render(GLuint shader);

private:
	GLuint VAO, VBO, EBO;
	BLOCK_TYPE type;
	glm::vec3 pos;
};
