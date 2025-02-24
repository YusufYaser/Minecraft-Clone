#pragma once
#include <glm/glm.hpp>
#include <glad/gl.h>

struct EntityModel {
	GLuint VAO;
};

enum ENTITY_TYPE {
	PLAYER = 0
};

EntityModel* getEntityModel(ENTITY_TYPE& type);

#define ENTITY_MODEL_COUNT 1

class Entity {
public:
	glm::vec3 getPos() const { return pos; };
	void setPos(glm::vec3 newPos) { pos = newPos; };

	glm::vec3 getOrientation() const { return orientation; };
	void setOrientation(glm::vec3 newOrientation) { orientation = newOrientation; };

	glm::vec3 getUp() const { return up; };

	void teleport(glm::vec3 newPos, glm::vec3 newOrientation) {
		pos = newPos;
		orientation = newOrientation;
	}

	void render();
	virtual void update() { assert("No update function set to this entity"); };
	void physicsUpdate(float delta);

protected:
	glm::vec3 pos = glm::vec3();
	glm::vec3 orientation = glm::vec3(1.0f, 0, 0);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	float verticalVelocity = 0.0f;
	bool flying = false;

	ENTITY_TYPE type;
};
