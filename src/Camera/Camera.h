#pragma once

#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include "../GameWindow/GameWindow.h"

class Camera {
public:
	Camera(glm::vec3 pos);

	glm::vec3 orientation = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	glm::vec3 pos;
	float speed = 2.0f;

	void checkInputs(GLFWwindow* window, float delta);

private:
	bool wasHidden = false;
	glm::vec2 originalPos;
};
