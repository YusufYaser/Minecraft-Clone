#include <iostream>
#include "Logging.h"
#include "config.h"
#include <csignal>
#include <thread>
#include <atomic>
#include "Game/Game.h"
#ifdef _WIN32
#undef APIENTRY
#include <Windows.h>
#endif

inline bool ctrlC = false;

inline Game* game = nullptr;

inline void signalHandler(int signal) {
	if (signal == SIGINT) {
		ctrlC = true;
	}
}

inline size_t maxMemory;

size_t getMaxMemory() {
	return maxMemory;
}

inline char* emergencyMemory;
inline std::atomic<bool> oom = false;
inline std::thread::id mainThread;

void oomHandler() {
	std::thread::id id = std::this_thread::get_id();
	if (!oom.load()) free(emergencyMemory);
	if (id == mainThread) {
		std::cerr << id << " (main): Out of game memory" << std::endl;
	} else {
		std::cerr << id << ": Out of game memory" << std::endl;
	}
	if (oom.load()) {
		if (mainThread != id) {
			std::this_thread::sleep_for(std::chrono::seconds(999));
		}
		return;
	}
	oom.store(true);
#ifdef _WIN32
	MessageBoxW(nullptr, L"The game ran out of memory. Try reducing your maximum render distance.", L"Out of Memory", MB_OK | MB_ICONERROR);
#endif
	if (mainThread != id) {
		std::this_thread::sleep_for(std::chrono::seconds(999));
	} else {
		std::abort();
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
	emergencyMemory = (char*)malloc(1024 * 1024 * 16);
	mainThread = std::this_thread::get_id();

#if defined(_WIN32)
	maxMemory = 4294967296;

	std::set_new_handler(oomHandler);

	HANDLE job = CreateJobObject(NULL, NULL);
	if (job == NULL) {
		error("Failed to set memory limit");
		return 1;
	}

	JOBOBJECT_EXTENDED_LIMIT_INFORMATION jobLimit;
	ZeroMemory(&jobLimit, sizeof(jobLimit));
	jobLimit.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_PROCESS_MEMORY;
	jobLimit.ProcessMemoryLimit = getMaxMemory();
	if (!SetInformationJobObject(job, JobObjectExtendedLimitInformation, &jobLimit, sizeof(jobLimit))) {
		error("Failed to set memory limit");
		CloseHandle(job);
		return 1;
	} else {
		AssignProcessToJobObject(job, GetCurrentProcess());
	}
#endif

#ifdef _DEBUG
	warn("Warning: You are running a debug build of the game");
	warn("         Some stuff such as chunk loading will be slow.");
#endif

	game = new Game();
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
		while (!game->shouldQuit() && !ctrlC && !oom) {
			game->update();
		}
		if (oom) {
			std::this_thread::sleep_for(std::chrono::seconds(30));
			return 2;
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
