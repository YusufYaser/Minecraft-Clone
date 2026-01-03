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
#include "Entity.h"

constexpr auto PLAYER_SPEED = 5.0f;
constexpr auto PLAYER_RUN_SPEED = 7.5f;
constexpr auto PLAYER_LIQUID_SPEED = 3.0f;

struct ItemStack {
	BLOCK_TYPE block;
};

class Player : public Entity {
public:
	Player();
	~Player();

	float speed = PLAYER_SPEED;
	float reachDistance = 7.0f;

	void update() override;

	void getTargetBlock(Block** block, BLOCK_FACE* face = nullptr);
	glm::vec3 getCameraPos() const { return pos + up; };

	glm::mat4 getProjection() const;
	glm::mat4 getView() const;

	bool isFlying() const { return flying; }
	void setFlying(bool nFlying) { flying = nFlying; }

	bool isChangingItem() const { return m_changingItem; };
	bool setChangingItem(bool nChangingItem) { m_changingItem = nChangingItem; };
	std::string getChangingItemInputString();
	int getChangingItemInputInt();

	bool isPerspective() const { return m_isPerspective; };
	void setPerspective(bool mPerspective) { m_isPerspective = mPerspective; };

	bool inFreecam() const { return freecam; };
	glm::vec3 getFreecamPos() const { return freecamPos; };
	glm::vec3 getFreecamOrientation() const { return freecamOrientation; };

private:
	bool wasHidden = false;
	bool freecam = false;
	glm::vec3 freecamPos = glm::vec3();
	glm::vec3 freecamStartPos = glm::vec3();
	glm::vec3 freecamOrientation = glm::vec3(1.0f, 0, 0);

	bool m_isPerspective = true;

	void checkInputs(float delta);

	Image* m_crosshair = nullptr;
	Image* m_inventory = nullptr;
	Image* m_inventoryImage;
	int slot = 0;

	ItemStack* m_items[9];

	bool m_changingItem = false;
	char changingItemInputs[2] = { '\0', '\0' };
};
