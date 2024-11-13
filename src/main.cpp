#include <iostream>
#include "Logging.h"
#include "Game/Game.h"

void glfw_error_callback(int error, const char* description) {
	std::cerr << "GLFW | Error | " << description << std::endl;
}

int main(int argc, char* argv[]) {

	int renderDistance = 0;

	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "--render-distance") {
			if (i + 1 < argc) {
				std::string value = argv[i + 1];
				if (renderDistance != 0) {
					error("Two or more seeds were specified");
					return 1;
				}
				try {
					renderDistance = std::stoi(value);
				}
				catch (std::invalid_argument&) {
					error("The render distance must be a number");
					return 1;
				}
				i++;
				print("Using render distance:", renderDistance);
				continue;
			}
			else {
				error("No render distance specified in --render-distance");
				return 1;
			}
		}
	}

	print("");
	print("Minecraft Clone");
	print("");
	print("GitHub:", UNDERLINE "https://github.com/YusufYaser/Minecraft-Clone");
	print("");

	print("Initializing GLFW");

	glfwSetErrorCallback(glfw_error_callback);

	if (!glfwInit()) {
		error("Failed to initialize GLFW");
		return 1;
	}

	print("Initialized GLFW");

	print("Starting game");
	Game* game = new Game();
	if (!game->successfullyLoaded()) {
		error("Failed to start game");
		return 1;
	}
	game->setRenderDistance(renderDistance == 0 ? 6 : renderDistance);

	double prevTime = glfwGetTime();

	while (!glfwWindowShouldClose(game->getGameWindow()->getGlfwWindow()))
	{
		double currentTime = glfwGetTime();
		float delta = float(currentTime - prevTime);
		prevTime = currentTime;

		game->update(delta);
	}
	print("Stopping game");

	print("Cleaning up");

	delete game;
	game = nullptr;

	print("Terminating GLFW");
	glfwTerminate();
	return 0;
}
