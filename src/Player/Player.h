#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include "../GameWindow/GameWindow.h"
#include "../Block/Block.h"
#include "../World/World.h"
#include <GLFW/glfw3.h>

#define PLAYER_SPEED 5.0f

class Player {
public:
	Player(World* world, glm::vec3 pos);

	glm::vec3 orientation = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	glm::vec3 pos;
	float speed = PLAYER_SPEED;
	float reachDistance = 10.0f;

	BLOCK_TYPE selectedBlock = BLOCK_TYPE::STONE;

	void update(float delta);
	void checkInputs(GLFWwindow* window, float delta);

	void getTargetBlock(Block** block, BLOCK_FACE* face);
	glm::vec3 getCameraPos();

private:
	bool wasHidden = false;
	bool brokeBlock = false;
	bool placedBlock = false;
	glm::vec2 originalPos;
	World* world;

	float jumpSpeed = 0.0f;
	float fallSpeed = 0.0f;
};
