#include "Game.h"
#include "ShaderSources.h"
#ifdef _WIN32
#undef APIENTRY
#include <Windows.h>
#undef max
#undef min
#endif

Game* Game::_instance = nullptr;

void logGlfwError(int error_code, const char* desc) {
	error("GLFW Error:", desc);

#ifdef _WIN32
	switch (error_code) {
	case GLFW_API_UNAVAILABLE:
	case GLFW_VERSION_UNAVAILABLE:
	case GLFW_PLATFORM_ERROR:
		MessageBox(nullptr, L"Your computer doesn't support the game's OpenGL version, or doesn't support OpenGL at all.", L"Error", MB_OK | MB_ICONERROR);
	}
#endif
}

Game::Game() {
	if (_instance != nullptr) {
		error("You cannot initialize 2 game instances");
		return;
	}
	_instance = this;

	print("");
	print("Minecraft Clone");
	print("");
	print("GitHub:", UNDERLINE "https://github.com/YusufYaser/Minecraft-Clone");
	print("");

	print("Loading settings");
	{
		GameSettings* settings = new GameSettings();
		if (std::filesystem::exists("settings.dat")) {
			std::ifstream settingsFile("settings.dat");
			settingsFile.read(reinterpret_cast<char*>(settings), sizeof(GameSettings));
			settingsFile.close();
		}
		m_maxFps = settings->maxFps;
		m_renderDistance = settings->renderDistance;
		m_worldRes = settings->worldRes;
		delete settings;
	}
	print("Loaded settings");

	print("Initializing GLFW");

	if (!glfwInit()) {
		error("Failed to initialize GLFW");
		return;
	}
	glfwSetErrorCallback(logGlfwError);

	print("Initialized GLFW");


#ifndef _DEBUG
	m_gameWindow = new GameWindow({ 854, 480 }, "Minecraft Clone");
#else
	m_gameWindow = new GameWindow({ 854, 480 }, "Minecraft Clone | Debug Build");
#endif

	if (m_gameWindow->getGlfwWindow() == NULL) {
		error("Failed to create game window");
		return;
	}

	const GPUInfo* gpu = getGpuInfo();
	print("Using GPU:", gpu->renderer);
	print("OpenGL and Driver Version:", gpu->version);

	// initialize shaders
	print("Intializing shaders");
	shader = new Shader(vertexShaderFile, fragmentShaderFile, "DefaultShader");
	guiShader = new Shader(guiVertexShaderFile, guiFragmentShaderFile, "GUI");
	skyboxShader = new Shader(skyboxVertexShaderFile, skyboxFragmentShaderFile, "Skybox");
	postProcessingShader = new Shader(postProcessingVertexShaderFile, postProcessingFragmentShaderFile, "PostProcessing");
	if (!shader->successfullyLoaded() || !guiShader->successfullyLoaded() || !skyboxShader->successfullyLoaded() || !postProcessingShader->successfullyLoaded()) {
		error("Failed to initialize shaders");
		return;
	}
	print("Intialized shaders");

	// load textures
	print("Loading textures");
	m_texAtlas = initializeTextures();

	shader->activate();
	shader->setUniform("tex0", 0);

	shader->activate();
	for (int i = 0; i < BLOCK_TYPE_COUNT; i++) {
		std::string name = "atlasRanges[" + std::to_string(i) + "]";
		glUniform4fv(glGetUniformLocation(shader->getId(), name.c_str()), 1, glm::value_ptr(m_texAtlas->ranges[i]));
	}

	guiShader->activate();
	guiShader->setUniform("tex0", 0);
	skyboxShader->activate();
	skyboxShader->setUniform("tex0", 0);
	postProcessingShader->activate();
	postProcessingShader->setUniform("tex0", 0);
	print("Loaded textures");

	print("Initializing sound engine");
	m_soundEngine = new SoundEngine();
	if (m_soundEngine->isLoaded()) {
		print("Initialized sound engine");

		print("Loading sounds");
		m_soundEngine->loadSounds();
		print("Loaded sounds");
	} else {
		error("Failed to initialize sound engine");

#ifdef _WIN32
		MessageBox(nullptr, L"The game has failed to initialize the sound engine.\nThe game will start without any sounds.", L"Sound engine failed to initialize", MB_OK | MB_ICONWARNING);
#endif
	}

	// load structures
	print("Loading structures");
	Structure::initialize();
	print("Loaded structures");

	// initialize other stuff
	gltInit();

	m_collOverlay = new Image(getTexture("stone"));
	m_collOverlay->setPosition({ .5f, 0, .5f, 0 });
	m_collOverlay->setSize({ 1, 0, 1, 0 });
	m_collOverlay->setColor({ .25f, .25f, .25f, 1.0f });
	m_collOverlay->setZIndex(2);

	m_flyingText = new Text();
	m_flyingText->setText("Flying");
	m_flyingText->setCentered(true);
	m_flyingText->setPosition({ 1.0f, -40, 1.0f, -15 });
	m_flyingText->setColor({ .75f, .75f, .75f, 1.0f });

	m_commandsHelp = new Text();
	m_commandsHelp->setText(R"(Commands

Q: Toggle 3D World Rendering
R: Teleport to 0, )" + std::to_string(MAX_HEIGHT) + R"(, 0
T: Teleport to height )" + std::to_string(MAX_HEIGHT) + R"(
Y: Reset world time
U: Reload shaders
)");
	m_commandsHelp->setPosition({ 0, 0, 1.0f, -17 * 7 });
	m_commandsHelp->setColor({ 1.0f, 1.0f, 1.0f, 1.0f });

	m_keyHandler = new KeyHandler();

	m_pauseMenu = new PauseMenu();
	m_mainMenu = new MainMenu();

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0, 0, 0, 1.0f);

	glGenFramebuffers(1, &worldFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, worldFBO);

	worldTex = new Texture();
	glGenTextures(1, &worldTex->id);
	glBindTexture(GL_TEXTURE_2D, worldTex->id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 854, 480, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, worldTex->id, 0);

	glGenFramebuffers(1, &worldFBOp);
	glBindFramebuffer(GL_FRAMEBUFFER, worldFBOp);

	worldTexp = new Texture();
	glGenTextures(1, &worldTexp->id);
	glBindTexture(GL_TEXTURE_2D, worldTexp->id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 854, 480, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, worldTexp->id, 0);

	worldImg = new Image(worldTexp);
	worldImg->setZIndex(1);
	worldImg->setSize({ 1.0f, 0, 1.0f, 0 });
	worldImg->setPosition({ .5f, 0, .5f, 0 });

	GLfloat vertices[] = {
		-1.0f, -1.0f,		0.0f, 0.0f,
		 1.0f, -1.0f,		1.0f, 0.0f,
		 1.0f,  1.0f,		1.0f, 1.0f,
		-1.0f, -1.0f,		0.0f, 0.0f,
		 1.0f,  1.0f,		1.0f, 1.0f,
		 -1.0f,  1.0f,		0.0f, 1.0f,
	};

	glGenVertexArrays(1, &worldVAO);
	glBindVertexArray(worldVAO);

	glGenBuffers(1, &worldVBO);
	glBindBuffer(GL_ARRAY_BUFFER, worldVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	DebugText::initialize();

	m_successfullyLoaded = true;
}

Game::~Game() {
	if (m_world != nullptr) unloadWorld();

	print("Saving settings");
	try {
		GameSettings* settings = new GameSettings();
		settings->maxFps = m_maxFps;
		settings->renderDistance = m_renderDistance;
		settings->worldRes = m_worldRes;

		std::ofstream settingsFile("settings.dat");
		settingsFile.write(reinterpret_cast<const char*>(settings), sizeof(GameSettings));
		settingsFile.close();
		print("Saved settings");
	} catch (std::filesystem::filesystem_error e) {
		error("FAILED TO SAVE GAME SETTINGS:", e.what());
	}

	delete m_keyHandler;
	m_keyHandler = nullptr;

	delete m_player;
	m_player = nullptr;

	delete m_world;
	m_world = nullptr;

	delete m_collOverlay;
	m_collOverlay = nullptr;

	delete m_flyingText;
	m_flyingText = nullptr;

	delete m_soundEngine;
	m_soundEngine = nullptr;

	delete shader;
	shader = nullptr;

	delete guiShader;
	guiShader = nullptr;

	delete skyboxShader;
	skyboxShader = nullptr;

	delete postProcessingShader;
	postProcessingShader = nullptr;

	delete m_gameWindow;
	m_gameWindow = nullptr;

	delete m_pauseMenu;
	m_pauseMenu = nullptr;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glDeleteFramebuffers(1, &worldFBO);
	glDeleteRenderbuffers(1, &worldRBO);
	glDeleteFramebuffers(1, &worldFBOp);
	glDeleteRenderbuffers(1, &worldRBOp);

	delete worldTex;
	worldTex = nullptr;

	delete worldTexp;
	worldTexp = nullptr;

	delete worldImg;
	worldImg = nullptr;

	glDeleteBuffers(1, &worldVBO);
	glDeleteVertexArrays(1, &worldVAO);

	DebugText::cleanup();

	print("Terminating GLFW");
	glfwTerminate();
#ifdef _WIN32
	gltTerminate();
#endif

	_instance = nullptr;
}

void Game::quit() {
	m_shouldQuit = true;
}

bool Game::shouldQuit() {
	return m_shouldQuit || glfwWindowShouldClose(getGlfwWindow());
}

void Game::update() {
	double startTime = glfwGetTime();
	m_frameNum++;

	GLenum errorCode;
	bool err = false;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		err = true;
		std::string error;
		switch (errorCode) {
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		error("glGetError():", error);
	}
#ifdef _DEBUG
	if (err) __debugbreak();
#endif

	m_keyHandler->update();

	if (m_keyHandler->keyHeld(GLFW_KEY_SLASH)) {
		if (m_keyHandler->keyClicked(GLFW_KEY_Q)) {
			worldRenderingEnabled = !worldRenderingEnabled;
			if (worldRenderingEnabled) {
				print("Enabled world rendering");
			} else {
				warn("Disabled world rendering");
			}
		}

		if (m_keyHandler->keyClicked(GLFW_KEY_R)) {
			m_player->pos = { 0.0f, (float)MAX_HEIGHT, 0.0f };
		}
		if (m_keyHandler->keyClicked(GLFW_KEY_T)) {
			m_player->pos.y = (float)MAX_HEIGHT;
		}
		if (m_keyHandler->keyClicked(GLFW_KEY_Y)) {
			m_world->setTick(0);
		}

		if (m_keyHandler->keyClicked(GLFW_KEY_U)) {
			print("Reloading shaders");
			Shader* shader = new Shader(vertexShaderFile, fragmentShaderFile, "DefaultShader");
			Shader* guiShader = new Shader(guiVertexShaderFile, guiFragmentShaderFile, "GUI");
			Shader* skyboxShader = new Shader(skyboxVertexShaderFile, skyboxFragmentShaderFile, "Skybox");
			Shader* postProcessingShader = new Shader(postProcessingVertexShaderFile, postProcessingFragmentShaderFile, "PostProcessing");
			if (!shader->successfullyLoaded() || !guiShader->successfullyLoaded() || !skyboxShader->successfullyLoaded() || !postProcessingShader->successfullyLoaded()) {
				error("Failed to reload shaders");
			} else {
				delete this->shader;
				delete this->guiShader;
				delete this->skyboxShader;
				delete this->postProcessingShader;
				this->shader = shader;
				this->guiShader = guiShader;
				this->skyboxShader = skyboxShader;
				this->postProcessingShader = postProcessingShader;
				print("Reloaded shaders");
			}
		}
	}

	if (m_keyHandler->keyClicked(GLFW_KEY_F3)) {
		m_debugLevel = (m_debugLevel + 1) % 3;
	}

	if (m_keyHandler->keyClicked(GLFW_KEY_ESCAPE) && m_player != nullptr) {
		m_gamePaused = !m_gamePaused;

		if (!m_gamePaused) {
			glm::ivec2 size = m_gameWindow->getSize();

			glfwSetCursorPos(getGlfwWindow(), size.x / 2, size.y / 2);
		}
	}

	if (m_keyHandler->keyClicked(GLFW_KEY_F11) ||
		(m_keyHandler->keyHeld(GLFW_KEY_RIGHT_ALT) && m_keyHandler->keyClicked(GLFW_KEY_ENTER))) {
		m_gameWindow->setFullscreen(!m_gameWindow->isFullscreen());
	}

	if (m_keyHandler->keyClicked(GLFW_KEY_F1)) {
		m_guiEnabled = !m_guiEnabled;
	}

	if (m_world == nullptr) m_gamePaused = true;

	glfwSetInputMode(getGlfwWindow(), GLFW_CURSOR, m_gamePaused ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);

	m_gameWindow->update();

	skyboxShader->activate();
	if (m_world != nullptr) skyboxShader->setUniform("tick", m_world->getTick());
	shader->activate();
	shader->setUniform("time", startTime);
	shader->setUniform("renderDistance", m_renderDistance);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDepthRange(0, 1.0);

	glm::vec2 size = m_gameWindow->getSize();

	if (m_world != nullptr && m_player != nullptr) {
		static glm::ivec2 prevSize;
		static World* prevWorld;
		glm::ivec2 iSize = glm::ivec2(size * m_worldRes);
		bool changed = prevSize.x != iSize.x || prevSize.y != iSize.y || prevWorld != m_world;
		static double lastChanged = 0;

		if ((!m_gamePaused || changed) && worldRenderingEnabled) {
			glBindTexture(GL_TEXTURE_2D, worldTex->id);
			glBindFramebuffer(GL_FRAMEBUFFER, worldFBO);
			glBindRenderbuffer(GL_RENDERBUFFER, worldRBO);

			if (changed) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, iSize.x, iSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
				worldTex->width = iSize.x;
				worldTex->height = iSize.y;

				glBindTexture(GL_TEXTURE_2D, worldTexp->id);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, iSize.x, iSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
				worldTexp->width = iSize.x;
				worldTexp->height = iSize.y;

				// texture
				glBindTexture(GL_TEXTURE_2D, worldTex->id);
				if (worldRBO != 0) glDeleteRenderbuffers(1, &worldRBO);
				glGenRenderbuffers(1, &worldRBO);
				glBindRenderbuffer(GL_RENDERBUFFER, worldRBO);

				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, iSize.x, iSize.y);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, worldRBO);

				glBindTexture(GL_TEXTURE_2D, worldTexp->id);
				glBindFramebuffer(GL_FRAMEBUFFER, worldFBOp);
				if (worldRBOp != 0) glDeleteRenderbuffers(1, &worldRBOp);
				glGenRenderbuffers(1, &worldRBOp);
				glBindRenderbuffer(GL_RENDERBUFFER, worldRBOp);

				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, iSize.x, iSize.y);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, worldRBOp);


				glBindTexture(GL_TEXTURE_2D, worldTex->id);
				glBindFramebuffer(GL_FRAMEBUFFER, worldFBO);
				glBindRenderbuffer(GL_RENDERBUFFER, worldRBO);

				if (m_pauseMenu->onSettings()) lastChanged = startTime;
				prevSize = iSize;
				prevWorld = m_world;
			}

			if (m_worldRes != 1.0f) glViewport(0, 0, iSize.x, iSize.y);

			glClear(GL_DEPTH_BUFFER_BIT);
			m_world->render();

			glBindFramebuffer(GL_FRAMEBUFFER, worldFBOp);
			glBindRenderbuffer(GL_RENDERBUFFER, worldRBOp);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			postProcessingShader->activate();

			// uniform sampler2D tex0
			glBindTexture(GL_TEXTURE_2D, worldTex->id);
			// uniform float time
			postProcessingShader->setUniform("time", (float)glfwGetTime());
			// uniform ivec2 resolution
			postProcessingShader->setUniform("resolution", iSize);
			Block* upBlock = m_world->getBlock(glm::ivec3(glm::round(m_player->pos)) + glm::ivec3(0, 1, 0));
			// uniform bool underWater
			postProcessingShader->setUniform("underWater", upBlock != nullptr && upBlock->getType() == BLOCK_TYPE::WATER);


			glBindVertexArray(worldVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			if (m_worldRes != 1.0f) glViewport(0, 0, int(size.x), int(size.y));

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		} else {
			m_world->dontRender();
		}

		bool wasGuiEnabled = m_guiEnabled;
		m_guiEnabled = true;
		worldImg->setColor(glm::vec4(glm::vec3((m_gamePaused && (!m_pauseMenu->onSettings() || startTime - lastChanged > 1)) ? .5f : 1.0f), 1.0f));
		worldImg->render();
		m_guiEnabled = wasGuiEnabled;
	}

	guiShader->activate();

	if (m_player != nullptr && m_world != nullptr) {
		glm::ivec3 iPos = glm::round(m_player->pos);

		Block* upBlock = m_world->getBlock(iPos + glm::ivec3(0, 1, 0));
		if (upBlock != nullptr) {
			m_collOverlay->setTexture(getTexture(upBlock->getName()));
			m_collOverlay->setCrop({ 1.0f / getAnimationFrameCount(upBlock->getType()), 1.0f / 6.0f });
			if (!m_gamePaused) {
				m_collOverlay->setColor({ .25f, .25f, .25f, 1.0f });
			} else {
				m_collOverlay->setColor({ .125f, .125f, .125f, 1.0f });
			}
			bool wasGuiEnabled = m_guiEnabled;
			m_guiEnabled = true;
			m_collOverlay->render();
			m_guiEnabled = wasGuiEnabled;
		}
	}

	if (m_gamePaused) {
		if (m_player != nullptr) {
			m_pauseMenu->render();
		} else {
			m_mainMenu->render();
		}
	} else {
		if (m_world != nullptr && m_player != nullptr) {
			m_player->update(getSimDelta());
		}
	}

	if (m_player != nullptr && m_player->isFlying() && !m_gamePaused) m_flyingText->render();
	if (m_debugLevel != 0) DebugText::render();

	if (m_keyHandler->keyHeld(GLFW_KEY_SLASH)) {
		m_commandsHelp->render();
	}

	glfwSwapBuffers(getGlfwWindow());
	glfwPollEvents();

	double endTime = glfwGetTime();
	double targetFps = m_maxFps;

	if (!m_gameWindow->isFocused()) {
		m_gamePaused = true;
		if (targetFps > 5 || targetFps == 0) targetFps = 5;
	}

	m_realDelta = static_cast<float>(glfwGetTime() - startTime);

	// limit FPS
	while (targetFps != 0 && glfwGetTime() - startTime < 1.0 / targetFps);

	m_delta = static_cast<float>(glfwGetTime() - startTime);
}

const GPUInfo* Game::getGpuInfo() {
	static GPUInfo* gpu = nullptr;
	if (gpu == nullptr) {
		gpu = new GPUInfo();
		gpu->vendor = glGetString(GL_VENDOR);
		gpu->renderer = glGetString(GL_RENDERER);
		gpu->version = glGetString(GL_VERSION);
	}
	return gpu;
}

void Game::setGamePaused(bool paused) {
	m_gamePaused = paused;

	glm::ivec2 size = getGameWindow()->getSize();

	glfwSetCursorPos(getGlfwWindow(), size.x / 2, size.y / 2);
}

void Game::loadWorld(WorldSettings& settings, glm::vec3 playerPos, glm::vec3 playerOrientation, bool playerFlying) {
	if (m_loadingWorld) return;
	m_loadingWorld = true;
	worldRenderingEnabled = true;

	print("Creating world");

	if (settings.seed == 0) {
		unsigned int seed = 0;
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<unsigned int> dis(
			std::numeric_limits<unsigned int>::min(),
			std::numeric_limits<unsigned int>::max()
		);

		settings.seed = dis(gen);
	}
	print("World Seed:", settings.seed);

	std::thread t = std::thread([this](WorldSettings settings, glm::vec3 playerPos, glm::vec3 playerOrientation, bool playerFlying) {
		m_world = new World(settings);
		for (int x = -2; x < 2; x++) {
			for (int y = -2; y < 2; y++) {
				m_world->loadChunk(glm::ivec2(x, y), true);
			}
		}

#ifndef _DEBUG
		print("Waiting for spawn chunks");
		while (m_world->chunkLoadQueueCount() != 0) {}
		print("Loaded spawn chunks");
#endif
		print("Created world");
		if (shouldQuit()) return;

		m_player = new Player();
		if (playerPos.y == 0) {
			playerPos.y = static_cast<float>(m_world->getHeight({ playerPos.x, playerPos.z }));
		}
		m_player->pos = playerPos;
		m_player->orientation = playerOrientation;
		m_player->setFlying(playerFlying);

		m_loadingWorld = false;
		}, settings, playerPos, playerOrientation, playerFlying);

	t.detach();

}

void Game::unloadWorld() {
	try {
		if (m_player != nullptr) {
			print("Saving world data");
			std::filesystem::create_directory("worlds");
			std::filesystem::create_directory("worlds/" + m_worldName);

			std::ofstream outFile("worlds/" + m_worldName + "/world.dat", std::ios::binary);
			WorldSaveData* s = m_world->createWorldSaveData();
			outFile.write(reinterpret_cast<const char*>(s), sizeof(WorldSaveData));
			outFile.close();

			delete s;
			print("Saved world data");
		}
	} catch (std::filesystem::filesystem_error e) {
		error("FAILED TO SAVE WORLD:", e.what());
	}

	delete m_world;
	m_world = nullptr;

	delete m_player;
	m_player = nullptr;
}

const char* Game::getBuild() {
	static std::string build = "";
	static bool gotBuildNumber = false;
	if (gotBuildNumber) {
		return build.c_str();
	}

	std::stringstream oss;

#ifdef _WIN32
	oss << "W";
#endif
#ifdef __linux__
	oss << "L";
#endif	
#ifdef __APPLE__
	oss << "A";
#endif	

	const char* date = __DATE__;
	// year
	oss << date[9] << date[10];
	// month
	if (date[0] == 'J' && date[1] == 'a')		oss << "01";
	else if (date[0] == 'F')					oss << "02";
	else if (date[0] == 'M' && date[2] == 'r')	oss << "03";
	else if (date[0] == 'A' && date[1] == 'p')	oss << "04";
	else if (date[0] == 'M' && date[2] == 'y')	oss << "05";
	else if (date[0] == 'J' && date[2] == 'n')	oss << "06";
	else if (date[0] == 'J' && date[2] == 'l')	oss << "07";
	else if (date[0] == 'A' && date[1] == 'u')	oss << "08";
	else if (date[0] == 'S')					oss << "09";
	else if (date[0] == 'O')					oss << "10";
	else if (date[0] == 'N')					oss << "11";
	else if (date[0] == 'D')					oss << "12";
	// day
	if (date[4] == ' ') oss << "0";
	else oss << date[4];
	oss << date[5];

#ifdef _DEBUG
	oss << "D";
	const char* time = __TIME__;
	// hour
	oss << time[0] << time[1];
	// minute
	oss << time[3] << time[4];
#endif

	build = oss.str();
	gotBuildNumber = true;

	return build.c_str();
}
