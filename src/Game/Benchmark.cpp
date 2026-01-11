#include "Game.h"

size_t getMaxMemory();

bool Game::startBenchmark() {
	if (m_benchmarkRunning) {
		error("Benchmark is already running");
		return false;
	}

	std::filesystem::create_directory("benchmarks");

	time_t timestamp = time(nullptr);
	tm datetime;
#ifdef _WIN32
	localtime_s(&datetime, &timestamp);
#else
	localtime_r(&timestamp, &datetime);
#endif
	char name[21];

	strftime(name, sizeof(name), "%d %b %Y %H-%M-%S", &datetime);

	m_benchmarkFile.open("benchmarks/Benchmark " + std::string(name) + ".csv");

	m_benchmarkRunning = m_benchmarkFile.is_open();

	if (m_benchmarkRunning) {
		const GPUInfo* gpu = getGpuInfo();
		m_benchmarkFile << "# Game Benchmark\n";
		m_benchmarkFile << "# Build: " << getBuild() << "\n";
		m_benchmarkFile << "# GPU: " << gpu->renderer << "\n";
		m_benchmarkFile << "# OpenGL Version: " << gpu->version << " MB\n";
		m_benchmarkFile << "# Max Memory: " << round(getMaxMemory() / 1024 / 1024) << "\n";
		m_benchmarkFile << "Frame,Time,Delta Time,FPS,Memory Usage (MB),Chunks Rendered,Chunks Loaded,Instances Rendered,Blocks Rendered\n";
		print("Benchmark started");
	}

	return m_benchmarkRunning;
}

bool Game::updateBenchmark() {
	if (!m_benchmarkRunning) {
		error("Benchmark is not running");
		return false;
	}

	double time = glfwGetTime();

	if (time - m_lastBenchmarkUpdate < 0.1) {
		return false;
	}

	m_lastBenchmarkUpdate = time;

	int frame = getFrameNum();
	float deltaTime = getDelta();
	int fps = round(1 / deltaTime);
	int memoryUsage = getMemoryUsage();
	int chunksRendered = 0;
	int chunksLoaded = 0;
	int instancesRendered = 0;
	int blocksRendered = 0;

	World* world = Game::getWorld();
	if (world) {
		chunksRendered = world->chunksRendered();
		chunksLoaded = world->chunksLoaded() - world->chunkLoadQueueCount();
		instancesRendered = world->instancesRendered();
		blocksRendered = world->blocksRendered();
	}

	m_benchmarkFile
		<< frame << ","
		<< time << ","
		<< deltaTime << ","
		<< fps << ","
		<< memoryUsage << ","
		<< chunksRendered << ","
		<< chunksLoaded << ","
		<< instancesRendered << ","
		<< blocksRendered << "\n";

	return true;
}

bool Game::stopBenchmark() {
	if (!m_benchmarkRunning) {
		error("Benchmark is not running");
		return false;
	}

	m_benchmarkFile.close();

	m_benchmarkRunning = m_benchmarkFile.is_open();

	if (!m_benchmarkRunning) {
		print("Benchmark stopped");
	}

	return m_benchmarkRunning;
}
