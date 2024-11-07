#include "Block.h"
#include "CubeVertices.h"

glm::vec3 getBlockFaceDirection(BLOCK_FACE face) {
	switch (face) {
	case BLOCK_FACE::FRONT:
		return glm::vec3(0.0f, 0.0f, 1.0f);

	case BLOCK_FACE::BACK:
		return glm::vec3(0.0f, 0.0f, -1.0f);

	case BLOCK_FACE::TOP:
		return glm::vec3(0.0f, 1.0f, 0.0f);

	case BLOCK_FACE::BOTTOM:
		return glm::vec3(0.0f, -1.0f, 0.0f);

	case BLOCK_FACE::RIGHT:
		return glm::vec3(1.0f, 0.0f, 0.0f);

	case BLOCK_FACE::LEFT:
		return glm::vec3(-1.0f, 0.0f, 0.0f);

	default:
		return glm::vec3();
	}
}

Block::Block(BLOCK_TYPE type, glm::vec3 pos, uint8_t hiddenFaces)
{
	Block::type = type;
	Block::pos = pos;
	Block::hiddenFaces = hiddenFaces;
	
	updateVertices();
}

void Block::updateVertices() {
	if (VAO != 0) glDeleteVertexArrays(1, &VAO);
	if (VBO != 0) glDeleteBuffers(1, &VBO);
	if (EBO != 0) glDeleteBuffers(1, &EBO);

	GLfloat vertices[6 * 5 * 4] = {};
	GLuint indices[6 * 6] = {};

	faceCount = 0;

	for (int i = 0; i < 6; i++) {
		if ((hiddenFaces & (1 << i)) != 0) continue;

		for (int j = 0; j < 5 * 4; j++) {
			vertices[(faceCount * 5 * 4) + j] = blockVertices[(i * 5 * 4) + j];
		}
		for (int j = 0; j < 6; j++) {
			indices[(faceCount * 6) + j] = blockIndices[j] + (faceCount * 4);
		}

		faceCount++;
	}

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, faceCount * 5 * 4 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceCount * 6 * sizeof(GLuint), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Block::Render(GLuint shader)
{
	glUseProgram(shader);
	glUniform3fv(glGetUniformLocation(shader, "blockPos"), 1, glm::value_ptr(pos));

	glm::vec3 highlightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	if (highlighted) {
		highlightColor *= 1.5;
	}
	glUniform3fv(glGetUniformLocation(shader, "highlightColor"), 1, glm::value_ptr(highlightColor));

	glBindTexture(GL_TEXTURE_2D, getTexture(getName()));
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, sizeof(blockIndices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
}

const char* Block::getName()
{
	switch (type) {
	case BLOCK_TYPE::AIR:
		return "air";

	case BLOCK_TYPE::STONE:
		return "stone";

	case BLOCK_TYPE::GRASS:
		return "grass";

	case BLOCK_TYPE::DIRT:
		return "dirt";

	default:
		return "invalid";
	}
}

glm::vec3 Block::getPos()
{
	return pos;
}

Block::~Block()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}
