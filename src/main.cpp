#include <iostream>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "GameWindow/GameWindow.h"
#include "Logging.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <string>
#include "Camera/Camera.h"
#include "Block/Block.h"
#include "Shaders/Shaders.h"
#include "Textures/Textures.h"
#include "World/World.h"
#include "GUI/GUI.h"
#include <sstream>
#include <random>
#include <ctime>

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
	glUniform1i(glGetUniformLocation(shader.ID, "tex0"), 0);
	print("Loaded textures");

	print("Creating world");
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<siv::PerlinNoise::seed_type> dis(0, std::numeric_limits<siv::PerlinNoise::seed_type>::max());
	print("World Seed:", dis(gen));
	World world(dis(gen), glm::vec2(50, 50));
	print("Created world");

	Camera camera = Camera(&world, glm::vec3(.0f, 10.0f, .0f));
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
			newTitle << " | Position: (" << round(camera.pos.x * 100) / 100;
			newTitle << ", " << round(camera.pos.y * 100) / 100;
			newTitle << ", " << round(camera.pos.z * 100) / 100 << ")";
			newTitle << " | Screen Resolution (ratio): " << width << "x" << height;
			newTitle << " (" << round(((double)width / height) * 100) / 100 << ")";

			glfwSetWindowTitle(gameWindow.getWindow(), newTitle.str().c_str());
		}

		glClearColor(.3f, .3f, 1.0f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glDepthRange(0.01, 1.0);

		view = glm::lookAt(camera.pos, camera.pos + camera.orientation, camera.up);

		glUseProgram(shader.ID);

		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));

		camera.checkInputs(gameWindow.getWindow(), delta);

		world.Render(shader.ID);

		Block* targetBlock = nullptr;
		camera.getTargetBlock(&targetBlock, &face);

		if (oldHighlightedBlock != nullptr) oldHighlightedBlock->highlighted = false;
		oldHighlightedBlock = targetBlock;
		if (targetBlock != nullptr) targetBlock->highlighted = true;

		glDepthRange(0, 0.01);
		glUseProgram(guiShader.ID);
		float aspectRatio = static_cast<float>(width) / height;
		float orthoHeight = 1.0f;
		float orthoWidth = orthoHeight * aspectRatio;
		ortho = glm::ortho(-orthoWidth, orthoWidth, -orthoHeight, orthoHeight, -1.0f, 1.0f);
		glUniformMatrix4fv(glGetUniformLocation(guiShader.ID, "viewport"), 1, GL_FALSE, glm::value_ptr(ortho));

		crosshair.Render(guiShader.ID);

		glfwSwapBuffers(gameWindow.getWindow());
		glfwPollEvents();
	}

	print("Cleaning up");
	//world.~World();
	shader.~Shader();
	gameWindow.~GameWindow();

	print("Terminating GLFW");
	glfwTerminate();
	return 0;
}
