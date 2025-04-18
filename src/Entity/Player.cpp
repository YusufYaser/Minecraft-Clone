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

	type = ENTITY_TYPE::PLAYER;
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

void Player::update() {
	Game* game = Game::getInstance();
	float delta = game->getSimDelta();
	Entity::physicsUpdate(delta);

	checkInputs(delta);

	bool wasDebug = false;
	if (game->getDebugLevel() == 2) {
		wasDebug = true;
		game->setDebugLevel(0);
	}
	m_crosshair->render();
	m_inventory->render();

	for (int i = 0; i < 9; i++) {
		m_inventoryImages[i]->setColor(i == slot ? glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) : glm::vec4(.5f, .5f, .5f, 1.0f));
		m_inventoryImages[i]->render();
	}
	if (wasDebug) {
		game->setDebugLevel(2);
	}
}

void Player::checkInputs(float delta) {
	Game* game = Game::getInstance();
	World* world = game->getWorld();

	KeyHandler* keyHandler = game->getKeyHandler();
	GLFWwindow* window = game->getGlfwWindow();

	glm::vec3 orientation2 = glm::vec3(orientation.x, 0.0f, orientation.z);
	if (freecam) {
		orientation2 = freecamOrientation;
	}

	glm::vec3 change = glm::vec3();

#ifdef GAME_DEBUG
	if (keyHandler->keyClicked(GLFW_KEY_F4)) {
		freecam = !freecam;
		freecamStartPos = pos;
		freecamPos = pos;
		freecamOrientation = orientation;

		debug(freecam ? "Enabled" : "Disabled", "debug freecam");
	}
#endif

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
	if (!freecam) {
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
				if (verticalVelocity == 0.0f && !freecam) verticalVelocity = 7.0f;
			}
		}
	} else {
		if (keyHandler->keyHeld(GLFW_KEY_E)) {
			change += glm::normalize(glm::cross(glm::cross(orientation2, up), orientation2));
		}
		if (keyHandler->keyHeld(GLFW_KEY_Q)) {
			change += -glm::normalize(glm::cross(glm::cross(orientation2, up), orientation2));
		}
	}

	if (change != glm::vec3(0, 0, 0)) {
		change = glm::normalize(change) * delta * speed;
		if (flying) change *= 2.0f;
		else if (verticalVelocity > 0) change *= 1.1f;
		if (!freecam) {
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
		} else {
			change = glm::normalize(change) * delta * 30.0f;
		}
		if (!freecam) {
			pos += change;
			freecamPos = glm::vec3();
		} else {
			freecamPos += change;
		}
	}

	if (freecam && keyHandler->keyClicked(GLFW_KEY_C)) {
		pos = freecamPos;
		orientation = freecamOrientation;

		debug("Teleported player to freecam");
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

	if (keyHandler->mouseClicked(GLFW_MOUSE_BUTTON_MIDDLE) && targetBlock != nullptr) {
		BLOCK_TYPE type = targetBlock->getType();
		for (int i = 0; i < 9; i++) {
			if (m_items[i]->block == type) {
				slot = i;
				break;
			}
		}
	}

	if (targetBlock != nullptr && !freecam) {
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

	if (!game->getGameWindow()->isFocused()) return;

	glm::ivec2 size = game->getGameWindow()->getSize();
	float rotX = 120.0f * (float)(posY - size.y / 2) / size.y;
	float rotY = 120.0f * (float)(posX - size.x / 2) / size.x;

	float pitch = glm::degrees(glm::asin(glm::dot(orientation, up)));
	if (freecam) {
		pitch = glm::degrees(glm::asin(glm::dot(freecamOrientation, up)));
	}

	float maxPitch = 89.9f;
	if ((pitch - rotX) > maxPitch) {
		rotX = pitch - maxPitch;
	} else if ((pitch - rotX) < -maxPitch) {
		rotX = pitch + maxPitch;
	}

	glm::vec3 right = glm::normalize(glm::cross(orientation, up));
	glm::vec3 newOrientation = glm::rotate(orientation, glm::radians(-rotX), right);
	if (freecam) {
		right = glm::normalize(glm::cross(freecamOrientation, up));
		newOrientation = glm::rotate(freecamOrientation, glm::radians(-rotX), right);
	}

	newOrientation = glm::rotate(newOrientation, glm::radians(-rotY), up);

	if (!freecam) {
		orientation = glm::normalize(newOrientation);
	} else {
		freecamOrientation = glm::normalize(newOrientation);
	}

	glfwSetCursorPos(window, size.x / 2, size.y / 2);
}

void Player::getTargetBlock(Block** block, BLOCK_FACE* face) {
	Game* game = Game::getInstance();
	World* world = game->getWorld();

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
	Game* game = Game::getInstance();
	KeyHandler* keyHandler = game->getKeyHandler();
	glm::vec2 size = game->getGameWindow()->getSize();
	int renderDistance = game->getRenderDistance();
	float d = sqrt(2 * ((renderDistance * 8.0f) * (renderDistance * 8.0f))) + 4.0f;

	float zFar = renderDistance * 16.0f;

	if (!m_isPerspective) {
		float orthoSize = 15.0f;

		return glm::ortho(-(size.x / size.y) * orthoSize, (size.x / size.y) * orthoSize, -1.0f * orthoSize, 1.0f * orthoSize, 0.01f, zFar * 10.0f);
	}

	static bool wasRunning = false;
	static double toggledRunning;
	double currentTime = glfwGetTime();

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

	return glm::perspective(glm::radians(FOV), size.x / size.y, 0.01f, zFar);
}

glm::mat4 Player::getView() const {
	if (!freecam) {
		if (m_isPerspective) {
			return glm::lookAt(glm::vec3(), orientation, up);
		} else {
			return glm::lookAt(glm::vec3(100.0f), glm::vec3(), up);
		}
	} else {
		return glm::lookAt(freecamPos - (pos), freecamPos - (pos)+freecamOrientation, up);
	}
}
