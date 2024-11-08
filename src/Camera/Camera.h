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

class Camera {
public:
	Camera(World* world, glm::vec3 pos);

	glm::vec3 orientation = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	glm::vec3 pos;
	float speed = 10.0f;
	float reachDistance = 50.0f;

	BLOCK_TYPE selectedBlock = BLOCK_TYPE::STONE;

	void checkInputs(GLFWwindow* window, float delta);

	void getTargetBlock(Block** block, BLOCK_FACE* face);

private:
	bool wasHidden = false;
	bool brokeBlock = false;
	bool placedBlock = false;
	glm::vec2 originalPos;
	World* world;
};
