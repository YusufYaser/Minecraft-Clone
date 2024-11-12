#include <iostream>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "GameWindow/GameWindow.h"
#include "Logging.h"
#include <glm/glm.hpp>
#include <string>
#include "Player/Player.h"
#include "Block/Block.h"
#include "Shaders/Shaders.h"
#include "Textures/Textures.h"
#include "World/World.h"
#include "GUI/Crosshair.h"
#include "GUI/GUIBlock.h"
#include <sstream>
#include <random>
#include <ctime>
#include <chrono>
#define GLT_IMPLEMENTATION
#include <gltext/gltext.h>

const char* vertexShaderFile = {
#include "Shaders/DefaultShader/Shader.vert"
};

const char* fragmentShaderFile = {
#include "Shaders/DefaultShader/Shader.frag"
};

const char* guiVertexShaderFile = {
#include "Shaders/GUI/GUI.vert"
};

const char* guiFragmentShaderFile = {
#include "Shaders/GUI/GUI.frag"
};

void glfw_error_callback(int error, const char* description) {
	std::cerr << "GLFW | Error | " << description << std::endl;
}

int main(int argc, char* argv[]) {
	print("");
	print("Starting Minecraft Clone");
	print("");
	print("GitHub:", UNDERLINE "https://github.com/YusufYaser/Minecraft-Clone");
	print("");

	uint32_t seedArg = 0;

	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "--seed") {
			if (i + 1 < argc) {
				std::string newSeed = argv[i + 1];
				if (seedArg != 0) {
					error("Two or more seeds were specified");
					return 1;
				}
				try {
					seedArg = std::stoi(newSeed);
				}
				catch (std::invalid_argument&) {
					seedArg = static_cast<uint32_t>(std::hash<std::string>{}(newSeed));
				}
				i++;
				print("Using custom seed:", seedArg);
				continue;
			}
			else {
				error("No seed specified in --seed");
				return 1;
			}
		}
	}

	print("Initializing GLFW");

	glfwSetErrorCallback(glfw_error_callback);

	if (!glfwInit()) {
		error("Failed to initialize GLFW");
		return 1;
	}

	print("Initialized GLFW");

	GameWindow gameWindow(800, 600, "Minecraft Clone");
	if (gameWindow.getWindow() == NULL) {
		error("Failed to create game window");
		return 1;
	}

	print("Intializing shaders");

	Shader shader = Shader(vertexShaderFile, fragmentShaderFile);
	Shader guiShader = Shader(guiVertexShaderFile, guiFragmentShaderFile);
	if (shader.ID == 0 || guiShader.ID == 0) {
		error("Failed to initialize shaders");
		return 1;
	}

	print("Intialized shaders");

	print("Loading textures");
	initializeTextures();

	glUseProgram(shader.ID);
	glUniform1i(glGetUniformLocation(shader.ID, "tex0"), 0);
	glUseProgram(guiShader.ID);
	glUniform1i(glGetUniformLocation(guiShader.ID, "tex0"), 0);

	gltInit();
	print("Loaded textures");

	print("Creating world");
	siv::PerlinNoise::seed_type seed = seedArg;
	if (seedArg == 0) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<siv::PerlinNoise::seed_type> dis(
			std::numeric_limits<siv::PerlinNoise::seed_type>::min(),
			std::numeric_limits<siv::PerlinNoise::seed_type>::max()
		);
			
		seed = dis(gen);
	}
	print("World Seed:", seed);
	World world(seed, glm::ivec2(250, 250));
	print("Created world");

	Player player = Player(&world, glm::vec3(.0f, 10.0f, .0f));
	Crosshair crosshair;

	glm::mat4 rotation = glm::mat4(1.0f);
	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 ortho;

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	double prevTime = glfwGetTime();

	Block* oldHighlightedBlock = nullptr;
	BLOCK_FACE face;

	GUIBlock guiBlock;

	BLOCK_TYPE prevSelectedBlock = player.selectedBlock;

	GLTtext* text = gltCreateText();

	bool showDebugText = true;
	int frameNum = 0;

	bool isRunning = false;
	bool wasRunning = false;
	double startedRunning = glfwGetTime();

	while (!glfwWindowShouldClose(gameWindow.getWindow()))
	{
		frameNum++;
		double currentTime = glfwGetTime();
		float delta = float(currentTime - prevTime);
		prevTime = currentTime;

		float simDelta = delta;
		if (simDelta > .5f) simDelta = .5f;

		int width, height;
		glfwGetWindowSize(gameWindow.getWindow(), &width, &height);
		if (width == 0) width = 1;
		if (height == 0) height = 1; // prevent division by 0

		isRunning = player.speed > PLAYER_SPEED;
		if (isRunning && !wasRunning) {
			startedRunning = currentTime;
		}
		wasRunning = isRunning;
		float FOV = 45.0f;
		if (isRunning) {
			if (currentTime - startedRunning > .1f) {
				FOV += 5.0f * player.speed / PLAYER_SPEED;
			}
			else {
				FOV += (currentTime - startedRunning) * (5.0f * player.speed / PLAYER_SPEED) * 10.0f;
			}
		}
		projection = glm::perspective(glm::radians(FOV), (float)width / height, .1f, 1000.0f);
		glViewport(0, 0, width, height);

		glClearColor(.3f, .3f, 1.0f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glDepthRange(0.01, 1.0);

		view = glm::lookAt(player.getCameraPos(), player.getCameraPos() + player.orientation, player.up);

		glUseProgram(shader.ID);

		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));

		guiBlock.position = glm::vec2(-1.5f, 1.5f);
		guiBlock.scale = .15f;

		player.update(simDelta);
		player.checkInputs(gameWindow.getWindow(), simDelta);

		world.Render(shader.ID, player.getCameraPos());

		Block* targetBlock = nullptr;
		player.getTargetBlock(&targetBlock, &face);

		if (oldHighlightedBlock != nullptr) oldHighlightedBlock->highlighted = false;
		oldHighlightedBlock = targetBlock;
		if (targetBlock != nullptr) targetBlock->highlighted = true;

		glDepthRange(0, 0.01);
		glUseProgram(guiShader.ID);
		float aspectRatio = static_cast<float>(width) / height;
		float orthoHeight = 1.0f;
		float orthoWidth = orthoHeight * aspectRatio;
		ortho = glm::ortho(-orthoWidth, orthoWidth, -orthoHeight, orthoHeight, -5.0f, 5.0f);
		glUniformMatrix4fv(glGetUniformLocation(guiShader.ID, "viewport"), 1, GL_FALSE, glm::value_ptr(ortho));

		crosshair.Render(guiShader.ID);

		if (prevSelectedBlock != player.selectedBlock) {
			prevSelectedBlock = player.selectedBlock;
			guiBlock.setBlock(player.selectedBlock);
		}

		if (showDebugText) {
			std::stringstream newTitle;
			newTitle << "Minecraft Clone\n";
			newTitle << "github.com/YusufYaser/Minecraft-Clone\n\n";
			newTitle << "FPS: " << round(1 / delta) << " (" << delta << ")" << "\n";

			newTitle << "Screen Resolution (ratio): " << width << "x" << height;
			newTitle << " (" << round(((double)width / height) * 100) / 100 << ")\n\n";

			newTitle << "Position: " << round(player.pos.x * 100) / 100;
			newTitle << ", " << round(player.pos.y * 100) / 100;
			newTitle << ", " << round(player.pos.z * 100) / 100 << "\n\n";

			newTitle << "Chunks Loaded: " << world.chunksLoaded() - world.chunkLoadQueueCount() << "\n";
			newTitle << "Chunks Load Queue Count: " << world.chunkLoadQueueCount() << "\n\n";

			newTitle << "World Seed: " << world.getSeed() << "\n";

			gltSetText(text, newTitle.str().c_str());
			gltBeginDraw();
			gltColor(1.0f, 1.0f, 1.0f, 1.0f);
			gltDrawText2D(text, 0, 0, 1.0f);
			gltEndDraw();
		}
		else {
			guiBlock.Render(guiShader.ID);
		}

		glfwSwapBuffers(gameWindow.getWindow());
		glfwPollEvents();
	}

	print("Cleaning up");
	gltDeleteText(text);
	world.~World();
	shader.~Shader();
	guiShader.~Shader();
	gameWindow.~GameWindow();

	print("Terminating GLFW");
	gltTerminate();
	glfwTerminate();
	return 0;
}
