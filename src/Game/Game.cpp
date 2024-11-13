#include "Game.h"
#include "ShaderSources.h"

Game* Game::_instance = nullptr;

Game::Game()
{
	if (_instance != nullptr) {
		error("You cannot initialize 2 game instances");
		return;
	}
	_instance = this;

	m_gameWindow = new GameWindow(800, 600, "Minecraft Clone");
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
	glUniform1i(shader->getUniformLoc("tex0"), 0);
	print("Loaded textures");

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
	m_world = new World(seed, glm::ivec2(250, 250));
	print("Created world");

	// initialize other stuff
	m_player = new Player(glm::vec3(.0f, 10.0f, .0f));

	m_crosshair = new Crosshair();

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	gltInit();
	m_debugText = gltCreateText();

	m_successfullyLoaded = true;
}

Game::~Game()
{
	delete m_gameWindow;
	m_gameWindow = nullptr;

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

	gltDeleteText(m_debugText);
	gltTerminate();

	_instance = nullptr;
}

Game* Game::getInstance()
{
	return _instance;
}

void Game::update(float delta)
{
	m_delta = delta;
	double currentTime = glfwGetTime();

	// change window size
	static glm::vec2 oldSize = glm::vec2();
	glm::vec2 size = m_gameWindow->getSize();
	if (size.x == 0) size.x = 1;
	if (size.y == 0) size.y = 1;
	if (size.x != oldSize.y || size.y != oldSize.y) {
		glViewport(0, 0, size.x, size.y);
		gltViewport(size.x, size.y);
	}
	oldSize = size;

	// get running FOV

	glClearColor(.3f, .3f, 1.0f, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDepthRange(0.01, 1.0);

	glm::mat4 projection = m_player->getProjection();

	m_player->update(getSimDelta());
	m_player->checkInputs(getGlfwWindow(), getSimDelta());

	glm::mat4 view = m_player->getView();
	shader->activate();
	glUniformMatrix4fv(shader->getUniformLoc("view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(shader->getUniformLoc("projection"), 1, GL_FALSE, glm::value_ptr(projection));

	Block* targetBlock = nullptr;
	m_player->getTargetBlock(&targetBlock);

	if (targetBlock != nullptr) targetBlock->highlighted = true;

	m_world->Render(shader);

	if (targetBlock != nullptr) targetBlock->highlighted = false;

	// render GUI
	glDepthRange(0, 0.01);
	guiShader->activate();
	float aspectRatio = size.x / size.y;
	float orthoHeight = 1.0f;
	float orthoWidth = orthoHeight * aspectRatio;
	glm::mat4 ortho = glm::ortho(-orthoWidth, orthoWidth, -orthoHeight, orthoHeight, -5.0f, 5.0f);
	glUniformMatrix4fv(guiShader->getUniformLoc("viewport"), 1, GL_FALSE, glm::value_ptr(ortho));

	m_crosshair->Render(guiShader);

	std::stringstream newTitle;
	newTitle << "Minecraft Clone\n";
	newTitle << "https://github.com/YusufYaser/Minecraft-Clone\n\n";
	static double lastFpsUpdated = 0;
	static int lastFps = 0;
	if (currentTime - lastFpsUpdated > .25) {
		lastFps = round(1 / delta);
		lastFpsUpdated = currentTime;
	}
	newTitle << "FPS: " << lastFps << " (" << delta << ")" << "\n";

	newTitle << "Screen Resolution: " << size.x << "x" << size.y << "\n";

	newTitle << "Position: " << round(m_player->pos.x * 100) / 100;
	newTitle << ", " << round(m_player->pos.y * 100) / 100;
	newTitle << ", " << round(m_player->pos.z * 100) / 100 << "\n\n";

	newTitle << "Chunks Loaded: " << m_world->chunksLoaded() - m_world->chunkLoadQueueCount() << "\n";
	newTitle << "Chunks Load Queue Count: " << m_world->chunkLoadQueueCount() << "\n\n";

	newTitle << "World Seed: " << m_world->getSeed() << "\n\n";

	if (targetBlock != nullptr) {
		newTitle << "=== Target Block ===\n";
		newTitle << " Block ID: " << (int)targetBlock->getType() << "\n";
		newTitle << " Block Type: " << targetBlock->getName() << "\n";
		glm::vec3 bPos = targetBlock->getPos();
		newTitle << " Block Position: " << round(bPos.x * 100) / 100;
		newTitle << ", " << round(bPos.y * 100) / 100;
		newTitle << ", " << round(bPos.z * 100) / 100 << "\n";
		newTitle << "==================\n";
	}

	gltSetText(m_debugText, newTitle.str().c_str());
	gltBeginDraw();
	gltColor(1.0f, 1.0f, 1.0f, 1.0f);
	gltDrawText2D(m_debugText, 0, 0, 1.0f);
	gltEndDraw();

	glfwSwapBuffers(getGlfwWindow());
	glfwPollEvents();
}

GameWindow* Game::getGameWindow()
{
	return m_gameWindow;
}

GLFWwindow* Game::getGlfwWindow()
{
	return getGameWindow()->getGlfwWindow();
}

World* Game::getWorld()
{
	return m_world;
}

Player* Game::getPlayer()
{
	return m_player;
}

float Game::getSimDelta()
{
	return m_delta > .5f ? .5f : m_delta;
}

float Game::getDelta()
{
	return m_delta;
}

int Game::getRenderDistance()
{
	return m_renderDistance;
}

void Game::setRenderDistance(int newRenderDistance)
{
	m_renderDistance = newRenderDistance;
}

bool Game::successfullyLoaded()
{
	return m_successfullyLoaded;
}
