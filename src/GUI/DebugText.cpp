#include "DebugText.h"
#include "../Game/Game.h"

Text* DebugText::m_debugText;

void DebugText::initialize() {
	m_debugText = new Text();
}

void DebugText::cleanup() {
	delete m_debugText;
	m_debugText = nullptr;
}

void DebugText::render() {
	Game* game = Game::getInstance();
	double currentTime = glfwGetTime();
	float delta = game->getDelta();
	glm::ivec2 size = game->getGameWindow()->getSize();
	Player* player = game->getPlayer();
	World* world = game->getWorld();

	std::stringstream text;
	text << "Minecraft Clone";
	if (game->gamePaused()) {
		text << " (Paused)";
	}
	text << "\nBuild: " << game->getBuild() << "\n";
	static double lastFpsUpdated = 0;
	static int lastFps = 0;
	static float lastDelta = 0;
	if (currentTime - lastFpsUpdated > .25) {
		lastFps = static_cast<int>(round(1 / delta));
		lastDelta = delta;
		lastFpsUpdated = currentTime;
	}
	text << "FPS: " << lastFps << " (" << lastDelta << ")" << "\n\n";

	text << "Resolution: " << size.x << "x" << size.y << "\n";
	const GPUInfo* gpu = game->getGpuInfo();
	text << "GPU: " << gpu->renderer << " (" << gpu->vendor << ")" << "\n";
	text << "Version: " << gpu->version << "\n\n";

	if (player != nullptr) {
		text << "Position: " << round(player->pos.x * 100) / 100;
		text << ", " << round(player->pos.y * 100) / 100;
		text << ", " << round(player->pos.z * 100) / 100 << "\n";

		glm::ivec2 chunkPos = getPosChunk(player->pos);
		text << "Chunk: " << round(chunkPos.x * 100) / 100;
		text << ", " << round(chunkPos.y * 100) / 100 << "\n\n";
	}

	if (world != nullptr) {
		text << "Render Distance: " << game->getRenderDistance() << "\n";
		text << "Chunks Rendered: " << world->chunksRendered() << "\n";
		text << "Chunks Loaded: " << world->chunksLoaded() - world->chunkLoadQueueCount() << "\n";
		text << "Chunks Load Queue Count: " << world->chunkLoadQueueCount() << "\n\n";

		text << "World Name: " << game->getLoadedWorldName() << "\n";
		text << "World Time: " << world->getTime();
		text << " (Day " << floor(world->getTime() / 24000.0f) << ")\n";
		text << "World Seed: " << world->getSeed() << "\n\n";
	}

	if (player != nullptr && world != nullptr) {
		Block* targetBlock = nullptr;
		player->getTargetBlock(&targetBlock);

		if (targetBlock != nullptr) {
			text << "=== Target Block ===\n";
			text << " Block ID: " << (int)targetBlock->getType() << "\n";
			text << " Block Type: " << targetBlock->getName() << "\n";
			glm::vec3 bPos = targetBlock->getPos();
			text << " Block Position: " << round(bPos.x * 100) / 100;
			text << ", " << round(bPos.y * 100) / 100;
			text << ", " << round(bPos.z * 100) / 100 << "\n";
			text << " Ambient Light: " << world->getAmbientLight() << "\n";
			text << "==================\n";
		}
	}

	m_debugText->setText(text.str().c_str());
	m_debugText->render();
}
