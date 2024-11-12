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
	print("GitHub: https://github.com/YusufYaser/Minecraft-Clone");
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
	float progress = 0.0f;
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
	double lastFpsUpdate = glfwGetTime();

	Block* oldHighlightedBlock = nullptr;
	BLOCK_FACE face;

	GUIBlock guiBlock;

	BLOCK_TYPE prevSelectedBlock = player.selectedBlock;

	while (!glfwWindowShouldClose(gameWindow.getWindow()))
	{
		double currentTime = glfwGetTime();
		float delta = float(currentTime - prevTime);
		prevTime = currentTime;

		int width, height;
		glfwGetWindowSize(gameWindow.getWindow(), &width, &height);
		if (height == 0) height = 1; // prevent division by 0
		projection = glm::perspective(glm::radians(45.0f), (float)width / height, .1f, 1000.0f);
		glViewport(0, 0, width, height);

		if (currentTime - lastFpsUpdate > .25) {
			lastFpsUpdate = currentTime;
			std::stringstream newTitle;
			newTitle << "Minecraft Clone";
			newTitle << " | FPS: " << round(1 / delta);
			newTitle << " | Position: (" << round(player.pos.x * 100) / 100;
			newTitle << ", " << round(player.pos.y * 100) / 100;
			newTitle << ", " << round(player.pos.z * 100) / 100 << ")";
			newTitle << " | Screen Resolution (ratio): " << width << "x" << height;
			newTitle << " (" << round(((double)width / height) * 100) / 100 << ")";
			newTitle << " | " << round(progress * 100);
			//newTitle << " | " << ((oldHighlightedBlock != nullptr) ? (oldHighlightedBlock->hiddenFaces + 0) : 0);

			glfwSetWindowTitle(gameWindow.getWindow(), newTitle.str().c_str());
		}

		glClearColor(.3f, .3f, 1.0f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glDepthRange(0.01, 1.0);

		view = glm::lookAt(player.getCameraPos(), player.getCameraPos() + player.orientation, player.up);

		glUseProgram(shader.ID);

		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));

		guiBlock.position = glm::vec2(-1.5f, 1.5f);
		guiBlock.scale = .15f;

		player.update(delta);
		player.checkInputs(gameWindow.getWindow(), delta);

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

		guiBlock.Render(guiShader.ID);

		glfwSwapBuffers(gameWindow.getWindow());
		glfwPollEvents();
	}

	print("Cleaning up");
	world.~World();
	shader.~Shader();
	guiShader.~Shader();
	gameWindow.~GameWindow();

	print("Terminating GLFW");
	glfwTerminate();
	return 0;
}
