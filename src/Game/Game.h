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
#include "../SoundEngine/SoundEngine.h"

struct GameSettings {
#ifndef _DEBUG
	int renderDistance = 24;
#else
	int renderDistance = 6;
#endif
	int maxFps = 60;
	float worldRes = 1;
};

struct GPUInfo {
	const GLubyte* vendor;
	const GLubyte* renderer;
	const GLubyte* version;
};

class Game {
public:
	Game();
	~Game();
	static Game* getInstance() { return _instance; };

	void quit();
	bool shouldQuit();

	void update();

	GameWindow* getGameWindow() const { return m_gameWindow; };
	GLFWwindow* getGlfwWindow() const { return m_gameWindow->getGlfwWindow(); };
	World* getWorld() const { return m_world; };
	Player* getPlayer() const { return m_player; };
	KeyHandler* getKeyHandler() const { return m_keyHandler; };
	SoundEngine* getSoundEngine() const { return m_soundEngine; };

	TextureAtlas* getTexAtlas() const { return m_texAtlas; }

	const GPUInfo* getGpuInfo();

	Shader* getShader() const { return shader; };
	Shader* getGuiShader() const { return guiShader; };
	Shader* getSkyboxShader() const { return skyboxShader; };

	// If the game is paused, zero is returned
	// If the game is running under 10 FPS, this function will return the time delta for 10 FPS.
	// Otherwise, this function returns the real time delta
	float getSimDelta() const { return !m_gamePaused ? std::min(m_delta, 1.0f / 10) : 0; };
	// You should probably use getSimDelta() instead
	float getDelta() const { return m_delta; };
	// Gets the delta time without putting into account the FPS limiter
	// This shouldn't be used at all
	float getRealDelta() const { return m_realDelta; };
	int getFrameNum() const { return m_frameNum; }

	int getDebugLevel() const { return m_debugLevel; }

	bool isGuiEnabled() const { return m_guiEnabled || m_gamePaused; };

	bool gamePaused() const { return m_gamePaused; };
	void setGamePaused(bool paused);

	int getRenderDistance() const { return m_renderDistance; };
	void setRenderDistance(int newRenderDistance) { m_renderDistance = newRenderDistance; };
	int getMaxFps() const { return m_maxFps; }
	void setMaxFps(int maxFps) { m_maxFps = maxFps; }
	float getWorldResolution() const { return m_worldRes; };
	void setWorldResolution(float newWorldRes) { m_worldRes = newWorldRes; };

	void loadWorld(WorldSettings& settings, glm::vec3 playerPos = { 0, 0, 0 }, glm::vec3 playerOrientation = { 1, 0, 0 }, bool playerFlying = false);
	void unloadWorld();
	bool loadingWorld() const { return m_loadingWorld; };
	void setLoadedWorldName(std::string name) { m_worldName = name; };
	std::string getLoadedWorldName() const { return m_worldName; };

	bool successfullyLoaded() const { return m_successfullyLoaded; };

	const char* getBuild();

private:
	static Game* _instance;
	GameWindow* m_gameWindow = nullptr;

	int m_frameNum = 0;

#ifdef _DEBUG
	int m_debugLevel = 1;
#else
	int m_debugLevel = 0;
#endif

	bool m_shouldQuit = false;

	std::string m_worldName = "invalid";

	TextureAtlas* m_texAtlas;

	Text* m_commandsHelp = nullptr;

	KeyHandler* m_keyHandler = nullptr;
	Player* m_player = nullptr;
	Image* m_collOverlay = nullptr;
	Text* m_flyingText = nullptr;
	SoundEngine* m_soundEngine = nullptr;

	World* m_world = nullptr;
	GLuint worldFBO = 0;
	GLuint worldRBO = 0;
	GLuint worldVAO = 0;
	GLuint worldVBO = 0;
	GLuint worldFBOp = 0;
	GLuint worldRBOp = 0;
	Texture* worldTex = nullptr;
	Texture* worldTexp = nullptr;
	Image* worldImg = nullptr;
	bool worldRenderingEnabled = true;

	bool m_guiEnabled = true;

	float m_realDelta = 1.0f;
	float m_delta = 1.0f;

	int m_renderDistance = 6;
	int m_maxFps = 60;
	float m_worldRes = 1;

	bool m_gamePaused = false;

	bool m_loadingWorld = false;
	bool m_successfullyLoaded = false;

	Shader* shader = nullptr;
	Shader* guiShader = nullptr;
	Shader* skyboxShader = nullptr;
	Shader* postProcessingShader = nullptr;

	// gui
	PauseMenu* m_pauseMenu = nullptr;
	MainMenu* m_mainMenu = nullptr;
};
