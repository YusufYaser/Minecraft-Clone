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
#include "../GUI/MainMenu/MainMenu.h"
#include "../KeyHandler/KeyHandler.h"
#include <gltext/gltext.h>
#include <fstream>

struct GameSettings {
	int renderDistance = 6;
};

struct GPUInfo {
	const GLubyte* vendor;
	const GLubyte* renderer;
	const GLubyte* version;
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

	const GPUInfo* getGpuInfo();

	Shader* getShader() const { return shader; };
	Shader* getGuiShader() const { return guiShader; };

	// If the game is paused, zero is returned
	// If the game is running under 10 FPS, this function will return the time delta for 10 FPS.
	// Otherwise, this function returns the real time delta
	float getSimDelta() const { return !m_gamePaused ? std::min(m_delta, 1.0f / 10) : 0; };
	// You should probably use getSimDelta() instead
	float getDelta() const { return m_delta; };
	int getFrameNum() const { return m_frameNum; }

	bool gamePaused() const { return m_gamePaused; };
	void setGamePaused(bool paused);

	int getRenderDistance() const { return m_renderDistance; };
	void setRenderDistance(int newRenderDistance) { m_renderDistance = newRenderDistance; };

	void loadWorld(WorldSettings& settings, glm::vec3 playerPos = { 0, 0, 0 });
	void unloadWorld();
	bool loadingWorld() const { return m_loadingWorld; };
	void setLoadedWorldName(std::string name) { m_worldName = name; };

	bool successfullyLoaded() const { return m_successfullyLoaded; };

	const char* getBuild();

private:
	static Game* _instance;
	GameWindow* m_gameWindow = nullptr;

	int m_frameNum = 0;

	bool m_shouldQuit = false;

	std::string m_worldName = "invalid";

	KeyHandler* m_keyHandler = nullptr;
	Player* m_player = nullptr;
	World* m_world = nullptr;
	Image* m_crosshair = nullptr;
	Image* m_collOverlay = nullptr;

	float m_delta = 1.0f;
	int m_renderDistance = 6;

	bool m_gamePaused = false;

	bool m_loadingWorld = false;
	bool m_successfullyLoaded = false;

	Shader* shader = nullptr;
	Shader* guiShader = nullptr;

	// gui
	PauseMenu* m_pauseMenu = nullptr;
	MainMenu* m_mainMenu = nullptr;
#ifdef _DEBUG
	bool m_debugTextVisible = true;
#else
	bool m_debugTextVisible = false;
#endif
};
