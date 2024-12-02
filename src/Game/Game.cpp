#include "Game.h"
#include "ShaderSources.h"

Game* Game::_instance = nullptr;

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

	print("Initialized GLFW");

	m_renderDistance = settings.renderDistance;

	m_gameWindow = new GameWindow(854, 480, "Minecraft Clone");
	if (m_gameWindow->getGlfwWindow() == NULL) {
		error("Failed to create game window");
		return;
	}

	print("GPU:", getGpu());

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
	glUniform1i(shader->getUniformLoc("tex0"), 0);
	guiShader->activate();
	glUniform1i(guiShader->getUniformLoc("tex0"), 0);
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
	return (m_shouldQuit || glfwWindowShouldClose(getGlfwWindow())) && !m_loadingWorld;
}

void Game::update() {
	double startTime = glfwGetTime();

	m_keyHandler->update();

	if (m_keyHandler->keyClicked(GLFW_KEY_F3)) {
		m_debugTextVisible = !m_debugTextVisible;
	}

	if (m_keyHandler->keyClicked(GLFW_KEY_ESCAPE)) {
		m_gamePaused = !m_gamePaused;

		if (!m_gamePaused) {
			glm::ivec2 size = m_gameWindow->getSize();

			glfwSetCursorPos(getGlfwWindow(), size.x / 2, size.y / 2);
		}
	}

	if (m_world == nullptr) m_gamePaused = true;

	glfwSetInputMode(getGlfwWindow(), GLFW_CURSOR, m_gamePaused ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);

	m_gameWindow->update();

	if (!m_gamePaused) {
		if (m_world != nullptr && m_player != nullptr) {
			m_player->update(getSimDelta());
		}
	}

	shader->activate();
	glUniform1i(shader->getUniformLoc("gamePaused"), m_gamePaused);
	if (m_gamePaused) {
		if (m_player != nullptr) {
			glClearColor(.3f * .5f, .3f * .5f, 1.0f * .5f, 1);
		} else {
			glClearColor(0, .25f, 0, 1);
		}
	} else {
		glClearColor(.3f, .3f, 1.0f, 1);
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDepthRange(0.01, 1.0);

	glm::vec2 size = m_gameWindow->getSize();

	if (m_world != nullptr) {
		m_world->render(shader);
	}

	glDepthRange(0, 0.01);
	guiShader->activate();

	if (!m_gamePaused) m_crosshair->render();

	if (m_gamePaused) {
		if (m_player != nullptr) {
			m_pauseMenu->render();
		} else {
			m_mainMenu->render();
		}
	}

	if (m_debugTextVisible) DebugText::render();

	glfwSwapBuffers(getGlfwWindow());
	glfwPollEvents();

	double endTime = glfwGetTime();
	double targetFps = 60;

	if (glfwGetWindowAttrib(getGlfwWindow(), GLFW_FOCUSED) == GLFW_FALSE) {
		m_gamePaused = true;
		if (targetFps > 5) targetFps = 5;
	}

	// limit FPS
	while (targetFps != 0 && glfwGetTime() - startTime < 1.0 / targetFps);

	m_delta = static_cast<float>(glfwGetTime() - startTime);
}

const GLubyte* Game::getGpu()
{
	static const GLubyte* gpu = nullptr;
	if (gpu == nullptr) {
		gpu = glGetString(GL_RENDERER);
	}
	return gpu;
}

void Game::setGamePaused(bool paused) {
	m_gamePaused = paused;

	glm::ivec2 size = getGameWindow()->getSize();

	glfwSetCursorPos(getGlfwWindow(), size.x / 2, size.y / 2);
}

void Game::loadWorld() {
	if (m_loadingWorld) return;
	m_loadingWorld = true;

	print("Creating world");
	siv::PerlinNoise::seed_type seed = 0;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<siv::PerlinNoise::seed_type> dis(
		std::numeric_limits<siv::PerlinNoise::seed_type>::min(),
		std::numeric_limits<siv::PerlinNoise::seed_type>::max()
	);

	seed = dis(gen);
	print("World Seed:", seed);
	
	std::thread t = std::thread([this](siv::PerlinNoise::seed_type seed) {
		WorldSettings worldSettings;
		worldSettings.seed = seed;

		m_world = new World(worldSettings);
		for (int x = -2; x < 2; x++) {
			for (int y = -2; y < 2; y++) {
				m_world->loadChunk(glm::ivec2(x, y), true);
			}
		}

		print("Waiting for spawn chunks");
		while (m_world->chunkLoadQueueCount() != 0) {}
		print("Loaded spawn chunks");
		print("Created world");

		m_player = new Player();
		m_player->pos = { .0f, m_world->getHeight({ 0, 0 }), .0f };

		m_gamePaused = false;
		m_loadingWorld = false;
	}, seed);

	t.detach();

}

void Game::unloadWorld() {
	delete m_world;
	m_world = nullptr;

	delete m_player;
	m_player = nullptr;
}
