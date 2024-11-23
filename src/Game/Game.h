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
#include "../GUI/PauseMenu.h"
#include "../KeyHandler/KeyHandler.h"
#define GLT_IMPORTS
#include <gltext/gltext.h>

struct GameSettings {
	int renderDistance = 6;
};

class Game {
public:
	Game(GameSettings& settings);
	~Game();
	static Game* getInstance() { return _instance; };

	void update(float delta);

	GameWindow* getGameWindow() { return m_gameWindow; };
	GLFWwindow* getGlfwWindow() { return m_gameWindow->getGlfwWindow(); };
	World* getWorld() { return m_world; };
	Player* getPlayer() { return m_player; };
	KeyHandler* getKeyHandler() { return m_keyHandler; };

	float getSimDelta() { return std::min(m_delta, .5f);  };
	// You should probably use getSimDelta() instead
	float getDelta() { return m_delta; };
	bool gamePaused() { return m_gamePaused; };

	int getRenderDistance() { return m_renderDistance; };
	void setRenderDistance(int newRenderDistance) { m_renderDistance = newRenderDistance; };

	bool successfullyLoaded() { return m_successfullyLoaded; };

private:
	static Game* _instance;
	GameWindow* m_gameWindow;

	KeyHandler* m_keyHandler;
	Player* m_player;
	World* m_world;
	Crosshair* m_crosshair;

	float m_delta = 1.0f;
	int m_renderDistance;

	bool m_gamePaused = false;

	bool m_successfullyLoaded = false;

	Shader* shader;
	Shader* guiShader;

	// gui
	PauseMenu* m_pauseMenu;
	#ifdef _DEBUG
	bool m_debugTextVisible = true;
	#else
	bool m_debugTextVisible = false;
	#endif
};
