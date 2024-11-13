#pragma once

#include "../World/World.h"
#include "../Player/Player.h"
#include "../Shaders/Shaders.h"
#include "../GUI/Crosshair.h"
#include "../GameWindow/GameWindow.h"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <sstream>

#define GLT_IMPLEMENTATION
#define GLT_MANUAL_VIEWPORT
#include <gltext/gltext.h>

class Game {
public:
	Game();
	~Game();
	static Game* getInstance();

	void update(float delta);

	GameWindow* getGameWindow();
	GLFWwindow* getGlfwWindow();
	World* getWorld();
	Player* getPlayer();

	float getSimDelta();
	// You should probably use getSimDelta() instead
	float getDelta();

	int getRenderDistance();
	void setRenderDistance(int newRenderDistance);

	bool successfullyLoaded();

private:
	static Game* _instance;
	GameWindow* m_gameWindow;

	Player* m_player;
	World* m_world;
	Crosshair* m_crosshair;

	float m_delta = 1.0f;
	int m_renderDistance = 6;

	bool m_successfullyLoaded = false;

	Shader* shader;
	Shader* guiShader;

	GLTtext* m_debugText;
};
