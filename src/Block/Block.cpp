#include "Block.h"
#include "CubeVertices.h"

Block::BlockStructureData Block::blockStructures[64];

glm::ivec3 getBlockFaceDirection(BLOCK_FACE face) {
	switch (face) {
	case BLOCK_FACE::FRONT:
		return glm::ivec3(0.0f, 0.0f, 1.0f);

	case BLOCK_FACE::BACK:
		return glm::ivec3(0.0f, 0.0f, -1.0f);

	case BLOCK_FACE::TOP:
		return glm::ivec3(0.0f, 1.0f, 0.0f);

	case BLOCK_FACE::BOTTOM:
		return glm::ivec3(0.0f, -1.0f, 0.0f);

	case BLOCK_FACE::RIGHT:
		return glm::ivec3(1.0f, 0.0f, 0.0f);

	case BLOCK_FACE::LEFT:
		return glm::ivec3(-1.0f, 0.0f, 0.0f);

	default:
		return glm::ivec3();
	}
}

const char* getTextureName(BLOCK_TYPE type)
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

	case BLOCK_TYPE::OAK_LOG:
		return "oak_log";

	case BLOCK_TYPE::OAK_LEAVES:
		return "oak_leaves";

	case BLOCK_TYPE::WATER:
		return "water";

	default:
		return "invalid";
	}
}

bool isBlockTypeTransparent(BLOCK_TYPE type)
{
	switch (type) {
	case BLOCK_TYPE::WATER:
	case BLOCK_TYPE::OAK_LEAVES:
		return true;

	default:
		return false;
	}
}

Block::Block(BLOCK_TYPE type, glm::ivec3 pos, uint8_t hiddenFaces)
{
	Block::type = type;
	Block::pos = pos;
	Block::hiddenFaces = hiddenFaces;
}

void Block::Render(Shader* shader, bool bindTexture)
{
	if (hiddenFaces == 63) return;

	GLuint VBO, EBO, VAO;
	uint8_t faceCount = 0;

	BlockStructureData data = blockStructures[hiddenFaces];

	if (data.VBO != 0)
	{
		VBO = data.VBO;
		VAO = data.VAO;
		EBO = data.EBO;
		faceCount = data.faceCount;
	}
	else {
		GLfloat* vertices = new GLfloat[6 * 5 * 4];
		GLuint* indices = new GLuint[6 * 6];

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

		delete[] vertices;
		delete[] indices;

		data.VBO = VBO;
		data.VAO = VAO;
		data.EBO = EBO;
		data.faceCount = faceCount;
		blockStructures[hiddenFaces] = data;
	}

	if (faceCount == 0) return;
	
	glUniform3iv(shader->getUniformLoc("blockPos"), 1, glm::value_ptr(pos));

	if (highlighted) {
		glUniform1i(shader->getUniformLoc("highlighted"), 1);
	}

	if (bindTexture) glBindTexture(GL_TEXTURE_2D, getTexture(getName()));

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, faceCount * 6, GL_UNSIGNED_INT, 0);

	if (highlighted) {
		glUniform1i(shader->getUniformLoc("highlighted"), 0);
	}
}

const char* Block::getName()
{
	return getTextureName(type);
}

glm::ivec3 Block::getPos()
{
	return pos;
}

BLOCK_TYPE Block::getType()
{
	return type;
}

bool Block::hasCollision()
{
	switch (type) {
	case BLOCK_TYPE::WATER:
		return false;

	default:
		return true;
	}
}

bool Block::hasTransparency()
{
	return isBlockTypeTransparent(type);
}
