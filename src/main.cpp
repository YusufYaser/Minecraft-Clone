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
#if defined(__linux__) || defined(__APPLE__)
#include <sys/resource.h>
#include <sys/sysinfo.h>
#include <dlfcn.h>
#endif
#ifdef __APPLE__
#include <sys/types.h>
#include <sys/sysctl.h>
#endif
#include <thread>

inline bool ctrlC = false;

inline Game* game = nullptr;

inline void signalHandler(int signal) {
	if (signal == SIGINT) {
		ctrlC = true;
	}
}

inline size_t maxMemory;
inline bool noSound;
inline std::string autoGotoWorld;

size_t getMaxMemory() {
	return maxMemory;
}

bool enableSound() {
	return !noSound;
}

std::string getAutoGotoWorld() {
	return autoGotoWorld;
}

inline void* emergencyMemory;
inline std::atomic<bool> oom = false;
inline std::thread::id mainThread;

#ifdef GAME_DEBUG
thread_local struct ThreadLogger {
	ThreadLogger() {
		if (oom.load()) return;

		if (std::this_thread::get_id() == mainThread) {
			debug("Main thread started");
		} else {
			debug("Thread started");
		}
	}
	~ThreadLogger() {
		if (oom.load()) return;

#ifdef _WIN32
		PWSTR desc;
		GetThreadDescription(GetCurrentThread(), &desc);
		std::wstring wstr;
		wstr.append(&desc[0]);
		debug("Thread exited:", std::string(wstr.begin(), wstr.end()));
#else
		debug("Thread exited");
#endif
	}
} threadLogger;
#endif

void oomHandler() {
	std::thread::id id = std::this_thread::get_id();
	if (!oom.load()) free(emergencyMemory);
	if (id == mainThread) {
		std::cerr << "Err (" << id << " (main)) -> Out of Game Memory" << std::endl;
	} else {
		std::cerr << "Err (" << id << ") -> Out of Game Memory" << std::endl;
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

	SetThreadDescription(GetCurrentThread(), L"Main and Renderer Thread");
#endif

	std::signal(SIGINT, signalHandler);
	emergencyMemory = (char*)malloc(1024 * 1024 * 16);
	mainThread = std::this_thread::get_id();
	std::set_new_handler(oomHandler);

#ifdef GAME_DEBUG
	std::string params = "";
	for (int i = 1; i < argc; i++) {
		params += std::string(argv[i]) + " ";
	}
	debug("Launch Parameters:");
	debug(params);
#endif

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--max-memory") == 0) {
			if (i + 1 >= argc) {
				error("No value provided with --max-memory");
				return 1;
			}

			size_t number = std::stoi(argv[++i]);
			maxMemory = number * 1024 * 1024;
		} else if (strcmp(argv[i], "--no-sound") == 0) {
			noSound = true;
		} else if (strcmp(argv[i], "--world") == 0) {
			if (i + 1 >= argc) {
				error("No value provided with --world");
				return 1;
			}

			autoGotoWorld = std::string(argv[++i]);
		} else if (strcmp(argv[i], "--load-library") == 0) {
			if (i + 1 >= argc) {
				error("No library path provided with --load-library");
				return 1;
			}

			std::string path = std::string(argv[++i]);

			print("Loading library", path);
#ifdef _WIN32
			HMODULE lib = LoadLibraryA(path.c_str());
			if (!lib) {
				error("Failed to load requested library:", GetLastError());
				return 1;
			}
#endif
#if defined(__linux__) || defined(__APPLE__)
			void* lib = dlopen(path.c_str(), RTLD_LAZY);
			if (!lib) {
				error("Failed to load requested library:", dlerror());
				return 1;
			}
#endif
		}
	}

#ifdef _WIN32
	if (maxMemory == 0) {
		MEMORYSTATUSEX memInfo;
		memInfo.dwLength = sizeof(MEMORYSTATUSEX);
		if (!GlobalMemoryStatusEx(&memInfo)) {
			error("Failed to set memory limit");
			return 1;
		}

		maxMemory = memInfo.ullTotalPhys / 2;
	}

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

#if defined(__linux__) || defined(__APPLE__)
#if defined(__linux__)
	if (maxMemory == 0) {
		struct sysinfo sysInfo;
		if (sysinfo(&sysInfo) != 0) {
			error("Failed to set memory limit");
			return 1;
		}

		maxMemory = sysInfo.totalram / 2;
	}
#elif defined(__APPLE__)
	if (maxMemory == 0) {
		size_t size = sizeof(maxMemory);
		if (sysctlbyname("hw.memsize", &maxMemory, &size, nullptr, 0) != 0) {
			error("Failed to set memory limit");
			return 1;
		}

		maxMemory = maxMemory / 2;
	}
#endif

	rlimit limit;
	limit.rlim_cur = maxMemory;
	limit.rlim_max = maxMemory;

#ifdef __APPLE__
	int resource = RLIMIT_DATA;
#else
	int resource = RLIMIT_AS;
#endif

	if (setrlimit(resource, &limit) != 0) {
		error("Failed to set memory limit");
		return 1;
	}
#endif

#ifdef GAME_DEBUG
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

#ifndef GAME_DEBUG
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
#ifndef GAME_DEBUG
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
