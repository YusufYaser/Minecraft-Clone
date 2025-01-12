#include <iostream>
#include "Logging.h"
#include <csignal>
#ifdef _WIN32
#undef APIENTRY
#include <Windows.h>
#endif
#include "Game/Game.h"

bool ctrlC = false;

void signalHandler(int signal) {
	if (signal == SIGINT) {
		ctrlC = true;
	}
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut != INVALID_HANDLE_VALUE) {
		DWORD dwMode = 0;
		if (GetConsoleMode(hOut, &dwMode)) {
			dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
			SetConsoleMode(hOut, dwMode);
		}
	}
#endif

	std::signal(SIGINT, signalHandler);

#ifdef _DEBUG
	warn("Warning: You are running a debug build of the game");
	warn("         Some stuff such as chunk loading will be slow.");
#endif

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
			} catch (std::invalid_argument&) {
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
#ifdef _WIN32
		MessageBox(nullptr, L"The game failed to start. Please check the game console for any errors.", L"Error", MB_OK | MB_ICONERROR);
#endif
		delete game;
		game = nullptr;
		return 1;
	}

#ifndef _DEBUG
	try {
#endif
		while (!game->shouldQuit() && !ctrlC) {
			game->update();
		}
		print("Stopping game");

		print("Cleaning up");

		delete game;
		game = nullptr;
#ifndef _DEBUG
	} catch (std::exception e) {
		error("The game has crashed!", e.what());
#ifdef _WIN32
		MessageBox(nullptr, L"The game has crashed, information about this crash is available in the game logs. Please try restarting the game", L"Game Crashed", MB_OK | MB_ICONERROR);
#endif
	} catch (...) {
		error("The game has crashed!");
#ifdef _WIN32
		MessageBox(nullptr, L"The game has crashed. Please try restarting the game", L"Game Crashed", MB_OK | MB_ICONERROR);
#endif
	}
#endif
	return 0;
}
