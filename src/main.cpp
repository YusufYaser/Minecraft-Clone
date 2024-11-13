#include <iostream>
#include "Logging.h"
#include "Game/Game.h"

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

	print("Starting game");
	Game* game = new Game();
	if (!game->successfullyLoaded()) {
		error("Failed to start game");
		return 1;
	}

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
	gltTerminate();
	glfwTerminate();
	return 0;
}
