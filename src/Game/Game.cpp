#include "Game.h"
#include "ShaderSources.h"

Game* Game::_instance = nullptr;

void logGlfwError(int error_code, const char* desc) {
	error("GLFW Error:", desc);
}

Game::Game(GameSettings& settings) {
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

	print("Initializing GLFW");

	if (!glfwInit()) {
		error("Failed to initialize GLFW");
		return;
	}
	glfwSetErrorCallback(logGlfwError);

	print("Initialized GLFW");

	m_renderDistance = settings.renderDistance;

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
	shader = new Shader(vertexShaderFile, fragmentShaderFile);
	guiShader = new Shader(guiVertexShaderFile, guiFragmentShaderFile);
	if (!shader->successfullyLoaded() || !guiShader->successfullyLoaded()) {
		error("Failed to initialize shaders");
		return;
	}
	print("Intialized shaders");

	// load textures
	print("Loading textures");
	initializeTextures();

	shader->activate();
	shader->setUniform("tex0", 0);
	guiShader->activate();
	guiShader->setUniform("tex0", 0);
	print("Loaded textures");

	// load structures
	print("Loading structures");
	Structure::initialize();
	print("Loaded structures");

	// initialize other stuff
	gltInit();

	m_crosshair = new Image(getTexture("crosshair"));
	m_crosshair->setPosition({ .5f, 0, .5f, 0 });
	m_crosshair->setSize({ 0, 16, 0, 16 });
	m_crosshair->setZIndex(999);

	m_collOverlay = new Image(getTexture("stone"));
	m_collOverlay->setPosition({ .5f, 0, .5f, 0 });
	m_collOverlay->setSize({ 1, 0, 1, 0 });
	m_collOverlay->setColor({ .25f, .25f, .25f, 1.0f });
	m_collOverlay->setZIndex(0);

	m_keyHandler = new KeyHandler();

	m_pauseMenu = new PauseMenu();
	m_mainMenu = new MainMenu();

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	DebugText::initialize();

	m_successfullyLoaded = true;
}

Game::~Game() {
	delete m_keyHandler;
	m_keyHandler = nullptr;

	delete m_player;
	m_player = nullptr;

	delete m_world;
	m_world = nullptr;

	delete m_crosshair;
	m_crosshair = nullptr;

	delete m_collOverlay;
	m_collOverlay = nullptr;

	delete shader;
	shader = nullptr;

	delete guiShader;
	guiShader = nullptr;

	delete m_gameWindow;
	m_gameWindow = nullptr;

	delete m_pauseMenu;
	m_pauseMenu = nullptr;

	DebugText::cleanup();

	print("Terminating GLFW");
	glfwTerminate();
	gltTerminate();

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

	m_keyHandler->update();

	if (m_keyHandler->keyClicked(GLFW_KEY_F3)) {
		m_debugTextVisible = !m_debugTextVisible;
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

	if (m_world == nullptr) m_gamePaused = true;

	glfwSetInputMode(getGlfwWindow(), GLFW_CURSOR, m_gamePaused ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);

	m_gameWindow->update();

	shader->activate();
	shader->setUniform("gamePaused", m_gamePaused);
	glm::vec3 clearColor = glm::vec3();
	if (m_gamePaused) {
		if (m_player != nullptr) {
			clearColor = { .3f, .3f, 1.0f };
			clearColor *= .5f;
		} else {
			clearColor = { 0, .25f, 0 };
		}
	} else {
		clearColor = { .3f, .3f, 1.0f };
	}
	if (m_world != nullptr && m_player != nullptr) {
		clearColor *= m_world->getAmbientLight();
	}
	glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDepthRange(0.01, 1.0);

	glm::vec2 size = m_gameWindow->getSize();

	if (m_world != nullptr) {
		m_world->render();
	}

	glDepthRange(0, 0.01);
	guiShader->activate();

	if (m_player != nullptr) {
		glm::ivec3 iPos = glm::ivec3(
			round(m_player->pos.x),
			round(m_player->pos.y),
			round(m_player->pos.z)
		);

		Block* upBlock = m_world->getBlock(iPos + glm::ivec3(0, 1, 0));
		if (upBlock != nullptr) {
			m_collOverlay->setTexture(getTexture(upBlock->getName()));
			if (!m_gamePaused) {
				m_collOverlay->setColor({ .25f, .25f, .25f, 1.0f });
			} else {
				m_collOverlay->setColor({ .125f, .125f, .125f, 1.0f });
			}
			m_collOverlay->render();
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

		m_crosshair->render();
	}

	if (m_debugTextVisible) DebugText::render();

	glfwSwapBuffers(getGlfwWindow());
	glfwPollEvents();

	double endTime = glfwGetTime();
#ifndef _DEBUG
	double targetFps = 60;
#else
	double targetFps = 0;
#endif

	if (!m_gameWindow->isFocused()) {
		m_gamePaused = true;
		if (targetFps > 5) targetFps = 5;
	}

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

void Game::loadWorld(WorldSettings& settings) {
	if (m_loadingWorld) return;
	m_loadingWorld = true;

	print("Creating world");

	if (settings.seed == 0) {
		siv::PerlinNoise::seed_type seed = 0;
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<siv::PerlinNoise::seed_type> dis(
			std::numeric_limits<siv::PerlinNoise::seed_type>::min(),
			std::numeric_limits<siv::PerlinNoise::seed_type>::max()
		);

		settings.seed = dis(gen);
	}
	print("World Seed:", settings.seed);

	std::thread t = std::thread([this](WorldSettings settings) {
		m_world = new World(settings);
		for (int x = -2; x < 2; x++) {
			for (int y = -2; y < 2; y++) {
				m_world->loadChunk(glm::ivec2(x, y), true);
			}
		}

		print("Waiting for spawn chunks");
		while (m_world->chunkLoadQueueCount() != 0) {}
		if (shouldQuit()) return;
		print("Loaded spawn chunks");
		print("Created world");

		m_player = new Player();
		m_player->pos = { .0f, m_world->getHeight({ 0, 0 }), .0f };

		m_gamePaused = false;
		m_loadingWorld = false;
		}, settings);

	t.detach();

}

void Game::unloadWorld() {
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
	oss << date[4] << date[5];

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
