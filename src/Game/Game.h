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
#include "../World/Structures.h"
#include "../World/Utils.h"
#include "../GUI/DebugText.h"

class Game {
public:
	Game();
	~Game();
	static Game* getInstance() { return _instance; };

	void update(float delta);

	GameWindow* getGameWindow() { return m_gameWindow; };
	GLFWwindow* getGlfwWindow() { return m_gameWindow->getGlfwWindow(); };
	World* getWorld() { return m_world; };
	Player* getPlayer() { return m_player; };

	float getSimDelta() { return m_delta > .5f ? .5f : m_delta;  };
	// You should probably use getSimDelta() instead
	float getDelta() { return m_delta; };

	int getRenderDistance() { return m_renderDistance; };
	void setRenderDistance(int newRenderDistance) { m_renderDistance = newRenderDistance; };

	bool successfullyLoaded() { return m_successfullyLoaded; };

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
};
