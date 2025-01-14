#include "Player.h"
#include "../Game/Game.h"

Player::Player() {
	m_crosshair = new Image(getTexture("crosshair"));
	m_crosshair->setPosition({ .5f, 0, .5f, 0 });
	m_crosshair->setSize({ 0, 16, 0, 16 });
	m_crosshair->setZIndex(999);

	m_inventory = new Image(getTexture("inventory"));
	m_inventory->setPosition({ .5f, 0, 1, -32 });
	m_inventory->setSize({ 0, 472, 0, 56 });
	m_inventory->setZIndex(998);

	for (int i = 0; i < 9; i++) {
		ItemStack* item = new ItemStack();
		item->block = BLOCK_TYPE(i + 2);
		m_items[i] = item;

		Image* img = new Image(getTexture(getTextureName(item->block)));
		img->setZIndex(999);
		img->setPosition({ .5f, -236 + (52 * i) + 28, 1, -32 });
		img->setSize({ 0, 48, 0, 48 });
		img->setCrop({ 1.0f / getAnimationFrameCount(item->block), 1.0f / 6.0f });
		m_inventoryImages[i] = img;
	}
}

Player::~Player() {
	delete m_crosshair;
	m_crosshair = nullptr;

	delete m_inventory;
	m_inventory = nullptr;

	for (int i = 0; i < 9; i++) {
		delete m_items[i];
		m_items[i] = nullptr;

		delete m_inventoryImages[i];
		m_inventoryImages[i] = nullptr;
	}
}

void Player::update(float delta) {
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
			if (feetBlock == nullptr || !feetBlock->hasCollision()) verticalVelocity -= 9.8f * delta;
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

	checkInputs(delta);

	m_crosshair->render();
	m_inventory->render();

	for (int i = 0; i < 9; i++) {
		m_inventoryImages[i]->setColor(i == slot ? glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) : glm::vec4(.5f, .5f, .5f, 1.0f));
		m_inventoryImages[i]->render();
	}
}

void Player::checkInputs(float delta) {
	Game* game = Game::getInstance();
	World* world = game->getWorld();

	KeyHandler* keyHandler = game->getKeyHandler();
	GLFWwindow* window = game->getGlfwWindow();

	glm::vec3 orientation2 = glm::vec3(orientation.x, 0.0f, orientation.z);

	if (keyHandler->keyHeld(GLFW_KEY_F3)) {
		if (keyHandler->keyHeld(GLFW_KEY_R)) {
			pos = { 0.0f, 100.0f, 0.0f };
		}
		if (keyHandler->keyHeld(GLFW_KEY_T)) {
			pos.y = 100.0f;
		}
		if (keyHandler->keyHeld(GLFW_KEY_Y)) {
			game->getWorld()->setTick(0);
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
	if (keyHandler->keyClicked(GLFW_KEY_F)) {
		flying = !flying;
	}
	if (flying) {
		verticalVelocity = 0;
		if (keyHandler->keyHeld(GLFW_KEY_SPACE)) {
			verticalVelocity += speed * 2.0f;
		}
		if (keyHandler->keyHeld(GLFW_KEY_LEFT_SHIFT)) {
			verticalVelocity += -speed * 2.0f;
		}
	} else {
		if (keyHandler->keyHeld(GLFW_KEY_SPACE)) {
			if (verticalVelocity == 0.0f) verticalVelocity = 6.0f;
		}
	}

	if (change != glm::vec3(0, 0, 0)) {
		change = glm::normalize(change) * delta * speed;
		if (flying) change *= 2.0f;
		else if (verticalVelocity > 0) change *= 1.1f;

		glm::vec3 aChange = glm::abs(change);
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
			block = world->getBlock({
				round(pos.x + change.x + (change.x > 0 ? .15f : -.15f)),
				round(pos.y + i),
				round(pos.z + change.z + (change.z > 0 ? .15f : -.15f))
				});
			if (block != nullptr && block->hasCollision()) {
				if (aChange.z > aChange.x) {
					change.x = 0;
				} else {
					change.z = 0;
				}
			}
		}
		pos += change;
	}

	if (keyHandler->keyHeld(GLFW_KEY_LEFT_CONTROL) && change != glm::vec3(0, 0, 0)) {
		speed = PLAYER_RUN_SPEED;
	}
	if (change == glm::vec3(0, 0, 0)) {
		speed = PLAYER_SPEED;
	}

	if (keyHandler->keyHeld(GLFW_KEY_1)) slot = 0;
	if (keyHandler->keyHeld(GLFW_KEY_2)) slot = 1;
	if (keyHandler->keyHeld(GLFW_KEY_3)) slot = 2;
	if (keyHandler->keyHeld(GLFW_KEY_4)) slot = 3;
	if (keyHandler->keyHeld(GLFW_KEY_5)) slot = 4;
	if (keyHandler->keyHeld(GLFW_KEY_6)) slot = 5;
	if (keyHandler->keyHeld(GLFW_KEY_7)) slot = 6;
	if (keyHandler->keyHeld(GLFW_KEY_8)) slot = 7;
	if (keyHandler->keyHeld(GLFW_KEY_9)) slot = 8;

	Block* targetBlock = nullptr;
	BLOCK_FACE face;
	getTargetBlock(&targetBlock, &face);

	if (targetBlock != nullptr) {
		static double lastModified = 0;
		double currentTime = glfwGetTime();

		if (keyHandler->mouseHeld(GLFW_MOUSE_BUTTON_RIGHT) && currentTime > lastModified + .2 && m_items[slot] != nullptr) {

			glm::ivec3 iPos = glm::ivec3(
				round(pos.x),
				round(pos.y),
				round(pos.z)
			);

			if (targetBlock->getPos() + getBlockFaceDirection(face) != iPos &&
				targetBlock->getPos() + getBlockFaceDirection(face) != iPos + glm::ivec3(up)) {

				Block* replacingBlock = world->getBlock(targetBlock->getPos() + getBlockFaceDirection(face));
				world->setBlock(targetBlock->getPos() + getBlockFaceDirection(face), m_items[slot]->block,
					replacingBlock != nullptr && replacingBlock->getType() == BLOCK_TYPE::WATER);

				lastModified = glfwGetTime();
			}
		}

		if (keyHandler->mouseHeld(GLFW_MOUSE_BUTTON_LEFT) && currentTime > lastModified + .2) {
			world->setBlock(targetBlock->getPos(), BLOCK_TYPE::AIR);
			getTargetBlock(&targetBlock, &face);
			lastModified = glfwGetTime();
		}

		if (!keyHandler->mouseHeld(GLFW_MOUSE_BUTTON_LEFT) && !keyHandler->mouseHeld(GLFW_MOUSE_BUTTON_RIGHT)) {
			lastModified = 0;
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
	World* world = Game::getInstance()->getWorld();

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

	Game* game = Game::getInstance();

	glm::vec2 size = game->getGameWindow()->getSize();
	int renderDistance = game->getRenderDistance();

	bool isRunning = speed == PLAYER_RUN_SPEED;
	if (isRunning && !wasRunning) {
		toggledRunning = currentTime;
	} else if (!isRunning && wasRunning) { // stopped running
		toggledRunning = currentTime;
	}
	wasRunning = isRunning;
	float FOV = 90.0f;
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

	float d = sqrt(2 * ((renderDistance * 8.0f) * (renderDistance * 8.0f))) + 4.0f;
	return glm::perspective(glm::radians(FOV), size.x / size.y, .1f, d > 1000.0f ? d : 1000.0f);
}

glm::mat4 Player::getView() const {
	return glm::lookAt(glm::vec3(), orientation, up);
}
