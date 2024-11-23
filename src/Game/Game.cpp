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

	// create world
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

	// initialize other stuff
	gltInit();

	m_player = new Player();
	m_player->pos = { .0f, m_world->getHeight({ 0, 0 }), .0f };

	m_crosshair = new Crosshair();
	m_keyHandler = new KeyHandler();

	m_pauseMenu = new PauseMenu();

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

	DebugText::cleanup();

	print("Terminating GLFW");
	glfwTerminate();
	gltTerminate();
	
	_instance = nullptr;
}

void Game::update(float delta) {
	m_delta = delta;

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

	if (glfwGetWindowAttrib(getGlfwWindow(), GLFW_FOCUSED) == GLFW_FALSE) {
		m_gamePaused = true;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

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
		glClearColor(.3f * .5f, .3f * .5f, 1.0f * .5f, 1);
	} else {
		glClearColor(.3f, .3f, 1.0f, 1);
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDepthRange(0.01, 1.0);

	glm::vec2 size = m_gameWindow->getSize();

	if (m_world != nullptr) {
		m_world->render(shader);

		glDepthRange(0, 0.01);
		guiShader->activate();
		float aspectRatio = size.x / size.y;
		float orthoHeight = 1.0f;
		float orthoWidth = orthoHeight * aspectRatio;
		glm::mat4 ortho = glm::ortho(-orthoWidth, orthoWidth, -orthoHeight, orthoHeight, -5.0f, 5.0f);
		glUniformMatrix4fv(guiShader->getUniformLoc("viewport"), 1, GL_FALSE, glm::value_ptr(ortho));

		if (!m_gamePaused) m_crosshair->render(guiShader);
	}

	if (m_gamePaused) {
		m_pauseMenu->render();
	}

	if (m_debugTextVisible) DebugText::render();

	glfwSwapBuffers(getGlfwWindow());
	glfwPollEvents();
}
