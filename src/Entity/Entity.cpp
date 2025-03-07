#include "Entity.h"
#include "../Block/Block.h"
#include "../Game/Game.h"
#include <glm/gtc/matrix_transform.hpp>

std::string getEntityName(ENTITY_TYPE& type) {
	switch (type) {
	case ENTITY_TYPE::PLAYER:
		return "player";

	default:
		return "invalid";
	}
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
	model = glm::rotate(model, static_cast<float>(atan2(lookDir.x, lookDir.z)), glm::vec3(0, 1.0f, 0));

	glm::mat4 view = glm::lookAt(player->getCameraPos(), player->getCameraPos() + player->getOrientation(), player->getUp());
	if (player->inFreecam()) {
		view = glm::lookAt(player->getFreecamPos() + player->getUp(), player->getFreecamPos() + player->getUp() + player->getFreecamOrientation(), player->getUp());
	}

	shader->setUniform("model", model);
	shader->setUniform("view", view);
	shader->setUniform("projection", player->getProjection());

	shader->setUniform("ambientLight", Game::getInstance()->getWorld()->getAmbientLight());

	glBindTexture(GL_TEXTURE_2D, getTexture(getEntityName(type))->id);
	glBindVertexArray(entityModel->VAO);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(entityModel->indicesCount), GL_UNSIGNED_INT, 0);
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
