#include <iostream>
#include "Logging.h"
#include "Game/Game.h"
#include <csignal>

bool ctrlC = false;

void signalHandler(int signal) {
	if (signal == SIGINT) {
		ctrlC = true;
	}
}

int main(int argc, char* argv[]) {
	std::signal(SIGINT, signalHandler);

	GameSettings settings;

	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "--render-distance") {
			if (i + 1 >= argc) {
				error("No render distance specified in --render-distance");
				return 1;
			}

			std::string value = argv[i + 1];
			try {
				settings.renderDistance = std::stoi(value);
			}
			catch (std::invalid_argument&) {
				error("The render distance must be a number");
				return 1;
			}
			i++;
			print("Using render distance:", settings.renderDistance);
			continue;
		} else {
			error("Invalid launch parameter:", arg);
			return 1;
		}
	}

	Game* game = new Game(settings);
	if (!game->successfullyLoaded()) {
		error("Failed to start game");
		delete game;
		game = nullptr;
		return 1;
	}

	double prevTime = glfwGetTime();

	while (!glfwWindowShouldClose(game->getGameWindow()->getGlfwWindow()) && !ctrlC)
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
	return 0;
}
