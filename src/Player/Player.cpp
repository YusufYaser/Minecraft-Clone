#include "Player.h"
#include "../Game/Game.h" 

Player::Player() {
	pos = glm::ivec3();
	world = Game::getInstance()->getWorld();
}

void Player::update(float delta) {
	glm::ivec3 iPos = glm::ivec3(
		round(pos.x),
		round(pos.y),
		round(pos.z)
	);

	if (jumpSpeed == 0.0f) {
		Block* belowBlock = world->getBlock(glm::vec3(iPos.x, pos.y, iPos.z) - up * delta);
		if (belowBlock == nullptr || !belowBlock->hasCollision()) {
			fallSpeed += 15.0f * delta;
			if (fallSpeed > 50.0f) fallSpeed = 50.0f;
		} else {
			fallSpeed = 0.0f;
			pos.y = belowBlock->getPos().y + 1.0f;
		}
	} else if (jumpSpeed != 0.0f) {
		Block* aboveBlock = world->getBlock(glm::vec3(iPos.x, pos.y + 1.8, iPos.z) + (jumpSpeed * delta) * up);
		if (aboveBlock == nullptr || !aboveBlock->hasCollision()) {
			pos += (jumpSpeed * delta) * up;
			jumpSpeed -= 15.0f * delta;
			if (jumpSpeed <= 0.0f) {
				jumpSpeed = 0.0f;
				fallSpeed = 15.0f * delta;
			}
		} else {
			jumpSpeed = 0.0f;
			fallSpeed = 15.0f * delta;
		}
	}

	pos -= (fallSpeed * delta) * up;

	if (fallSpeed == 0.0f) {
		Block* collBlock = world->getBlock(iPos);
		if (collBlock != nullptr && collBlock->hasCollision()) {
			Block* upBlock = world->getBlock(iPos + glm::ivec3(up));
			if (upBlock == nullptr) {
				pos.y = collBlock->getPos().y + 1.0f;
			}
		}
	}

	if (pos.y < -20.0f) {
		pos.y = 20.0f;
	}

	checkInputs(delta);
}

void Player::checkInputs(float delta) {
	KeyHandler* keyHandler = Game::getInstance()->getKeyHandler();
	GLFWwindow* window = Game::getInstance()->getGlfwWindow();

	glm::vec3 orientation2 = glm::vec3(orientation.x, 0.0f, orientation.z);

	if (keyHandler->keyHeld(GLFW_KEY_F3)) {
		if (keyHandler->keyHeld(GLFW_KEY_R)) {
			pos = { 0.0f, 30.0f, 0.0f };
		}
		if (keyHandler->keyHeld(GLFW_KEY_T)) {
			pos.y = 30.0f;
		}
	}

	glm::vec3 change = glm::vec3();

	if (keyHandler->keyHeld(GLFW_KEY_W)) {
		change += glm::normalize(orientation2);
	}
	if (keyHandler->keyHeld(GLFW_KEY_S)) {
		change += -glm::normalize(orientation2);
	}
	if (keyHandler->keyHeld(GLFW_KEY_A)) {
		change += -glm::normalize(glm::cross(orientation2, up));
	}
	if (keyHandler->keyHeld(GLFW_KEY_D)) {
		change += glm::normalize(glm::cross(orientation2, up));
	}
	if (keyHandler->keyHeld(GLFW_KEY_SPACE)) {
		if (fallSpeed == 0.0f && jumpSpeed == 0.0f) jumpSpeed = 5.5f;
	}

	if (change != glm::vec3(0, 0, 0)) {
		change = glm::normalize(change) * delta * speed;
		Block* block;
		for (int i = 0; i <= 1; i++) {
			block = world->getBlock({
				round(pos.x + change.x + (change.x > 0 ? .15f : -.15f)),
				round(pos.y + i),
				round(pos.z)
				});
			if (block != nullptr && block->hasCollision()) change.x = 0;
			block = world->getBlock({
				round(pos.x),
				round(pos.y + i),
				round(pos.z + change.z + (change.z > 0 ? .15f : -.15f))
				});
			if (block != nullptr && block->hasCollision()) change.z = 0;
		}
		pos += change;
	}

	if (keyHandler->keyHeld(GLFW_KEY_LEFT_CONTROL) && change != glm::vec3(0, 0, 0)) {
		speed = PLAYER_RUN_SPEED;
	}
	if (change == glm::vec3(0, 0, 0)) {
		speed = PLAYER_SPEED;
	}

	if (keyHandler->keyHeld(GLFW_KEY_1)) selectedBlock = BLOCK_TYPE::STONE;
	if (keyHandler->keyHeld(GLFW_KEY_2)) selectedBlock = BLOCK_TYPE::GRASS;
	if (keyHandler->keyHeld(GLFW_KEY_3)) selectedBlock = BLOCK_TYPE::DIRT;
	if (keyHandler->keyHeld(GLFW_KEY_4)) selectedBlock = BLOCK_TYPE::OAK_LOG;
	if (keyHandler->keyHeld(GLFW_KEY_5)) selectedBlock = BLOCK_TYPE::OAK_LEAVES;
	if (keyHandler->keyHeld(GLFW_KEY_6)) selectedBlock = BLOCK_TYPE::BEDROCK;
	if (keyHandler->keyHeld(GLFW_KEY_7)) selectedBlock = BLOCK_TYPE::SAND;
	if (keyHandler->keyHeld(GLFW_KEY_8)) selectedBlock = BLOCK_TYPE::OAK_PLANKS;

	if (keyHandler->mouseClicked(GLFW_MOUSE_BUTTON_LEFT)) {
		Block* target = nullptr;
		BLOCK_FACE face;
		getTargetBlock(&target, &face);

		if (target != nullptr) {
			world->setBlock(target->getPos(), BLOCK_TYPE::AIR);
		}
	}

	if (keyHandler->mouseClicked(GLFW_MOUSE_BUTTON_RIGHT)) {
		Block* target = nullptr;
		BLOCK_FACE face;
		getTargetBlock(&target, &face);

		if (target != nullptr) {
			glm::ivec3 iPos = glm::ivec3(
				round(pos.x),
				round(pos.y),
				round(pos.z)
			);

			if (target->getPos() + getBlockFaceDirection(face) != iPos &&
				target->getPos() + getBlockFaceDirection(face) != iPos + glm::ivec3(up)) {
				world->setBlock(target->getPos() + getBlockFaceDirection(face), selectedBlock, false);
			}
		}
	}

	double posX, posY;
	glfwGetCursorPos(window, &posX, &posY);

	if (!wasHidden) {
		wasHidden = true;
	}

	glm::ivec2 size = Game::getInstance()->getGameWindow()->getSize();
	float rotX = 100.0f * (float)(posY - size.y / 2) / size.y;
	float rotY = 100.0f * (float)(posX - size.x / 2) / size.x;

	float pitch = glm::degrees(glm::asin(glm::dot(orientation, up)));

	float maxPitch = 89.9f;
	if ((pitch - rotX) > maxPitch) {
		rotX = pitch - maxPitch;
	} else if ((pitch - rotX) < -maxPitch) {
		rotX = pitch + maxPitch;
	}

	glm::vec3 right = glm::normalize(glm::cross(orientation, up));
	glm::vec3 newOrientation = glm::rotate(orientation, glm::radians(-rotX), right);

	newOrientation = glm::rotate(newOrientation, glm::radians(-rotY), up);

	orientation = glm::normalize(newOrientation);

	glfwSetCursorPos(window, size.x / 2, size.y / 2);
}

void Player::getTargetBlock(Block** block, BLOCK_FACE* face) {
	glm::vec3 oldBlockPos = glm::vec3();
	glm::vec3 cameraPos = getCameraPos();
	for (float i = 0; i < reachDistance; i += .01f) {
		glm::vec3 blockPosNoCeil = cameraPos + i * orientation;
		glm::vec3 blockPos = cameraPos + i * orientation;
		blockPos.x = round(blockPos.x);
		blockPos.y = round(blockPos.y);
		blockPos.z = round(blockPos.z);
		if (blockPos == oldBlockPos) continue;
		oldBlockPos = blockPos;
		Block* targetBlock = world->getBlock(blockPos);
		if (targetBlock != nullptr && targetBlock->hasCollision()) {
			if (block != nullptr) *block = targetBlock;
			if (face != nullptr) {
				BLOCK_FACE curFace = BLOCK_FACE::FRONT;
				float distance = 1000.0f;

				for (int i = 0; i < 6; i++) {
					glm::ivec3 facePos = getBlockFaceDirection((BLOCK_FACE)i) + targetBlock->getPos();
					float curDistance = glm::distance(glm::vec3(facePos), blockPosNoCeil);
					if (curDistance < distance) {
						curFace = (BLOCK_FACE)i;
						distance = curDistance;
					}
				}

				*face = curFace;
			}
			return;
		}
	}
	*block = nullptr;
}

glm::mat4 Player::getProjection() const {
	static bool wasRunning = false;
	static double toggledRunning;
	double currentTime = glfwGetTime();

	glm::vec2 size = Game::getInstance()->getGameWindow()->getSize();

	bool isRunning = speed == PLAYER_RUN_SPEED;
	if (isRunning && !wasRunning) {
		toggledRunning = currentTime;
	} else if (!isRunning && wasRunning) { // stopped running
		toggledRunning = currentTime;
	}
	wasRunning = isRunning;
	float FOV = 45.0f;
	if (isRunning) {
		if (currentTime - toggledRunning > .1f) {
			FOV += 7.5f * speed / PLAYER_SPEED;
		} else {
			FOV += static_cast<float>(currentTime - toggledRunning) * (7.5f * speed / PLAYER_SPEED) * 10.0f;
		}
	} else if (currentTime - toggledRunning < .1f) {
		FOV += 7.5f * PLAYER_RUN_SPEED / PLAYER_SPEED;
		FOV -= static_cast<float>(currentTime - toggledRunning) * (7.5f * PLAYER_RUN_SPEED / PLAYER_SPEED) * 10.0f;
	}

	return glm::perspective(glm::radians(FOV), size.x / size.y, .1f, 1000.0f);
}

glm::mat4 Player::getView() const {
	return glm::lookAt(getCameraPos(), getCameraPos() + orientation, up);
}
