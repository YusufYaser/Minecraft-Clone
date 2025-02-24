#include "Entity.h"
#include "../Block/Block.h"
#include "../Game/Game.h"
#include <glm/gtc/matrix_transform.hpp>

inline EntityModel* models[ENTITY_MODEL_COUNT];

EntityModel* createModel(GLfloat* vertices, size_t verticesSize, GLuint* indices, size_t indicesSize) {
	GLuint VAO, VBO, EBO;

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, verticesSize, vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	EntityModel* model = new EntityModel();
	model->VAO = VAO;

	return model;
}

EntityModel* getEntityModel(ENTITY_TYPE& type) {
	if (models[(int)type] != nullptr) {
		return models[(int)type];
	}

	switch (type) {
	case ENTITY_TYPE::PLAYER:
	{
		GLfloat vertices[] = {
			// front face
			 0.3f, -0.5f,  0.3f,
			-0.3f, -0.5f,  0.3f,
			-0.3f,  1.5f,  0.3f,
			 0.3f,  1.5f,  0.3f,

			 // back face
			 -0.3f, -0.5f, -0.3f,
			 0.3f, -0.5f, -0.3f,
			 0.3f,  1.5f, -0.3f,
			 -0.3f,  1.5f, -0.3f,

			 // right face
			 0.3f, -0.5f, -0.3f,
			 0.3f, -0.5f,  0.3f,
			 0.3f,  1.5f,  0.3f,
			 0.3f,  1.5f, -0.3f,

			 // left face
			 -0.3f, -0.5f,  0.3f,
			 -0.3f, -0.5f, -0.3f,
			 -0.3f,  1.5f, -0.3f,
			 -0.3f,  1.5f,  0.3f,

			 // bottom face
			 -0.3f, -0.5f,  0.3f,
			 0.3f, -0.5f,  0.3f,
			 0.3f, -0.5f, -0.3f,
			 -0.3f, -0.5f, -0.3f,

			 // top face
			 0.3f,  1.5f,  0.3f,
			 -0.3f,  1.5f,  0.3f,
			 -0.3f,  1.5f, -0.3f,
			 0.3f,  1.5f, -0.3f,
		};

		GLuint indices[] = {
			0, 1, 2, 0, 2, 3,
			4, 5, 6, 4, 6, 7,
			8, 9, 10, 8, 10, 11,
			12, 13, 14, 12, 14, 15,
			16, 17, 18, 16, 18, 19,
			20, 21, 22, 20, 22, 23
		};

		models[(int)type] = createModel(vertices, sizeof(vertices), indices, sizeof(indices));
	}

	break;
	}

	return models[(int)type];
}

void Entity::render() {
	EntityModel* entityModel = getEntityModel(Entity::type);
	if (entityModel == nullptr) return;

	Player* player = Game::getInstance()->getPlayer();
	if (player == nullptr) return;

	Shader* shader = Game::getInstance()->getEntityShader();
	shader->activate();

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, pos);
	glm::vec3 lookDir = glm::normalize(glm::vec3(orientation.x, 0.0f, orientation.z));
	model = glm::rotate(model, atan2(lookDir.x, lookDir.z), { 0, 1.0f, 0 });

	glm::mat4 view = glm::lookAt(player->getCameraPos(), player->getCameraPos() + player->getOrientation(), player->getUp());
	if (player->inFreecam()) {
		view = glm::lookAt(player->getFreecamPos() + player->getUp(), player->getFreecamPos() + player->getUp() + player->getFreecamOrientation(), player->getUp());
	}

	shader->setUniform("model", model);
	shader->setUniform("view", view);
	shader->setUniform("projection", player->getProjection());

	shader->setUniform("ambientLight", Game::getInstance()->getWorld()->getAmbientLight());

	glBindVertexArray(entityModel->VAO);
	glDrawElements(GL_TRIANGLES, 6 * 6, GL_UNSIGNED_INT, 0);
}

void Entity::physicsUpdate(float delta) {
	World* world = Game::getInstance()->getWorld();

	glm::ivec3 iPos = glm::round(pos);
	iPos.y = static_cast<int>(floor(pos.y));

	Block* aboveBlock = world->getBlock(iPos + glm::ivec3(0, 1, 0));

	if (verticalVelocity > 0) {
		verticalVelocity -= 20.0f * delta;
	}

	if (verticalVelocity <= 0 && !flying) {
		verticalVelocity -= 20.0f * delta;
		if (verticalVelocity < -98.0f) verticalVelocity = -98.0f;
	}

	if (verticalVelocity != 0 && !world->isChunkLoaded(getPosChunk(pos))) {
		verticalVelocity = 0;
	}

	if (verticalVelocity < 0) {
		for (int i = iPos.y; i > floor(iPos.y + verticalVelocity * delta); i--) {
			Block* block = world->getBlock({ iPos.x, i, iPos.z });
			if (block == nullptr) continue;
			if (!block->hasCollision()) continue;

			verticalVelocity = 0;
			pos.y = block->getPos().y + 1.0f;

			break;
		}
	}

	if (verticalVelocity > 0) {
		Block* headBlock = world->getBlock({ iPos.x, pos.y + 1, iPos.z });
		Block* aboveBlock = world->getBlock({ iPos.x, pos.y + 1.8, iPos.z });
		Block* feetBlock = world->getBlock(iPos);

		if ((headBlock != nullptr && headBlock->hasCollision()) || (aboveBlock != nullptr && aboveBlock->hasCollision())) {
			verticalVelocity = 0;
			if ((feetBlock == nullptr || !feetBlock->hasCollision()) && !flying) verticalVelocity -= 9.8f * delta;
		}
	}

	pos.y += verticalVelocity * delta;

	iPos = glm::round(pos);
	iPos.y = static_cast<int>(floor(pos.y));

	if (verticalVelocity <= 0) {
		Block* collBlock = world->getBlock(iPos);
		if (collBlock != nullptr && collBlock->hasCollision()) {
			verticalVelocity = 0;
			pos.y = collBlock->getPos().y + 1.0f;
			iPos = glm::floor(pos);
			iPos.y = static_cast<int>(floor(pos.y));
		}
	}

	if (pos.y < -20.0f) {
		pos.y = 100.0f;
	}
}
