#pragma once

#include "../World/World.h"
#include "../Player/Player.h"
#include "../Shaders/Shaders.h"
#include "../GUI/Components/Image.h"
#include "../GameWindow/GameWindow.h"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <sstream>
#include "../World/Structures.h"
#include "../World/Utils.h"
#include "../GUI/DebugText.h"
#include "../GUI/PauseMenu.h"
#include "../GUI/MainMenu.h"
#include "../KeyHandler/KeyHandler.h"
#include <gltext/gltext.h>

struct GameSettings {
	int renderDistance = 6;
};

class Game {
public:
	Game(GameSettings& settings);
	~Game();
	static Game* getInstance() { return _instance; };

	void quit();
	bool shouldQuit();

	void update();

	GameWindow* getGameWindow() { return m_gameWindow; };
	GLFWwindow* getGlfwWindow() { return m_gameWindow->getGlfwWindow(); };
	World* getWorld() { return m_world; };
	Player* getPlayer() { return m_player; };
	KeyHandler* getKeyHandler() { return m_keyHandler; };

	const GLubyte* getGpu();

	Shader* getGuiShader() { return guiShader; };

	float getSimDelta() { return std::min(m_delta, .5f); };
	// You should probably use getSimDelta() instead
	float getDelta() { return m_delta; };

	bool gamePaused() { return m_gamePaused; };
	void setGamePaused(bool paused);

	int getRenderDistance() { return m_renderDistance; };
	void setRenderDistance(int newRenderDistance) { m_renderDistance = newRenderDistance; };

	void loadWorld();
	void unloadWorld();
	bool loadingWorld() { return m_loadingWorld; };

	bool successfullyLoaded() { return m_successfullyLoaded; };

private:
	static Game* _instance;
	GameWindow* m_gameWindow;

	bool m_shouldQuit = false;

	KeyHandler* m_keyHandler;
	Player* m_player;
	World* m_world;
	Image* m_crosshair;
	Image* m_collOverlay;

	float m_delta = 1.0f;
	int m_renderDistance;

	bool m_gamePaused = false;

	bool m_loadingWorld = false;
	bool m_successfullyLoaded = false;

	Shader* shader;
	Shader* guiShader;

	// gui
	PauseMenu* m_pauseMenu;
	MainMenu* m_mainMenu;
#ifdef _DEBUG
	bool m_debugTextVisible = true;
#else
	bool m_debugTextVisible = false;
#endif
};
