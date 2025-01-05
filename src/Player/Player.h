#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include "../Block/Block.h"
#include "../World/World.h"
#include <GLFW/glfw3.h>
#include "../GUI/Components/Image.h"

constexpr auto PLAYER_SPEED = 5.0f;
constexpr auto PLAYER_RUN_SPEED = 7.5f;

struct ItemStack {
	BLOCK_TYPE block;
};

class Player {
public:
	Player();
	~Player();

	glm::vec3 orientation = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	glm::vec3 pos = glm::vec3();
	float speed = PLAYER_SPEED;
	float reachDistance = 7.0f;

	void update(float delta);

	void getTargetBlock(Block** block, BLOCK_FACE* face = nullptr);
	glm::vec3 getCameraPos() const { return pos + up; };

	glm::mat4 getProjection() const;
	glm::mat4 getView() const;

private:
	bool wasHidden = false;

	float verticalVelocity = 0.0f;

	void checkInputs(float delta);

	Image* m_crosshair = nullptr;
	Image* m_inventory = nullptr;
	Image* m_inventoryImages[9];
	int slot = 0;

	ItemStack* m_items[9];
};
