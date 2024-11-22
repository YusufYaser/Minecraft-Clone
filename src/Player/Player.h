#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include "../Block/Block.h"
#include "../World/World.h"
#include <GLFW/glfw3.h>

#define PLAYER_SPEED 5.0f
#define PLAYER_RUN_SPEED 7.5f

class Player {
public:
	Player();

	glm::vec3 orientation = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	glm::vec3 pos;
	float speed = PLAYER_SPEED;
	float reachDistance = 10.0f;

	BLOCK_TYPE selectedBlock = BLOCK_TYPE::STONE;

	void update(float delta);

	void getTargetBlock(Block** block, BLOCK_FACE* face = nullptr);
	glm::vec3 getCameraPos() { return pos + up; };

	glm::mat4 getProjection();
	glm::mat4 getView();

private:
	bool wasHidden = false;
	World* world;

	float jumpSpeed = 0.0f;
	float fallSpeed = 0.0f;

	void checkInputs(float delta);
};
