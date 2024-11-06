#include "Block.h"

GLfloat vertices[] = {
	// front face
	-.5f, -.5f, .5f,	1.0f, 0.0f, 0.0f,
	.5f, -.5f, .5f,		0.0f, 1.0f, 0.0f,
	.5f, .5f, .5f,		0.0f, 0.0f, 1.0f,
	-.5f, .5f, .5f,		1.0f, 1.0f, 1.0f,

	// back face
	-.5f, -.5f, -.5f,	1.0f, 0.0f, 0.0f,
	.5f, -.5f, -.5f,	0.0f, 1.0f, 0.0f,
	.5f, .5f, -.5f,		0.0f, 0.0f, 1.0f,
	-.5f, .5f, -.5f,	1.0f, 1.0f, 1.0f,
};

GLuint indices[] = {
	// front face
	0, 1, 2,
	0, 2, 3,

	// back face
	4, 5, 6,
	4, 6, 7,

	// right face
	1, 5, 2,
	2, 6, 5,

	// left face
	0, 4, 3,
	3, 7, 4,

	// bottom face
	0, 4, 5,
	0, 5, 1,

	// top face
	3, 7, 6,
	3, 6, 2
};

Block::Block(BLOCK_TYPE type, glm::vec3 pos)
{
	Block::type = type;
	Block::pos = pos;

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Block::Render(GLuint shader)
{
	glUniform3fv(glGetUniformLocation(shader, "blockPos"), 1, glm::value_ptr(pos));
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
}

Block::~Block()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}
