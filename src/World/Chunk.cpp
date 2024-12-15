#include "World.h"
#include "Utils.h"

void World::chunkLoaderFunc() {
	while (true) {
		if (unloading.load()) break;

		if (chunkLoadQueue.size() == 0) continue;
		chunkLoadQueueMutex.lock();
		glm::ivec2 pos = chunkLoadQueue.front();
		chunkLoadQueue.erase(chunkLoadQueue.begin());
		chunkLoadQueueMutex.unlock();

		std::size_t chunkCh = hashPos(pos);
		chunksMutex.lock();
		Chunk* chunk = chunks[chunkCh];
		chunksMutex.unlock();

		if (generator == Generator::Void) {
			if (pos.x == 0 && pos.y == 0) {
				setBlock({ 0, 0, 0 }, BLOCK_TYPE::STONE);
			}
			chunk->loaded = true;
			continue;
		}

		for (int x = pos.x * 16; x < 16 + pos.x * 16; x++) {
			for (int z = pos.y * 16; z < 16 + pos.y * 16; z++) {
				if (unloading.load()) break;

				int height = getHeight({ x, z });
				for (int y = 0; y < height; y++) {
					if (unloading.load()) break;
					BLOCK_TYPE type = BLOCK_TYPE::STONE;
					if (y == 0) {
						type = BLOCK_TYPE::BEDROCK;
					} else if (height > 6) {
						if (y == height - 1) type = BLOCK_TYPE::GRASS;
						else if (y >= height - 3) type = BLOCK_TYPE::DIRT;
					} else {
						if (y == height - 1) type = BLOCK_TYPE::SAND;
						else type = BLOCK_TYPE::STONE;
					}

					setBlock(glm::ivec3(x, y, z), type);
				}
				if (height <= 6) {
					for (int y = height; y < 6; y++) {
						setBlock(glm::ivec3(x, y, z), BLOCK_TYPE::WATER);
					}
				}

				int MAX_STRUCTURE_HEIGHT = 0;

				std::vector<Structure*> structuresInChunk;

				for (Structure* structure : structures) {
					if (!structure->isInXZ({ x, z })) continue;

					int sHeight = structure->getHeight();
					sHeight -= structure->getPivot().y;
					sHeight += height;
					if (sHeight > MAX_STRUCTURE_HEIGHT) MAX_STRUCTURE_HEIGHT = sHeight;

					structuresInChunk.push_back(structure);
				}

				if (structuresInChunk.size() == 0) continue;

				for (int y = 0; y < height + MAX_STRUCTURE_HEIGHT; y++) {
					for (Structure* structure : structuresInChunk) {
						BLOCK_TYPE blockType = structure->getBlock({ x, y, z });
						if (blockType != BLOCK_TYPE::NONE) {
							setBlock(glm::ivec3(x, y, z), blockType);
							break;
						}
					}
				}
			}
		}

		chunk->loaded = true;
	}
}

void World::chunkUnloaderFunc() {
	while (true) {
		if (unloading.load()) break;
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		time_t current = time(nullptr);
		if (!chunksMutex.try_lock()) continue;
	unloader_func_chunks_loop:
		for (auto& [ch, chunk] : chunks) {
			if (chunk == nullptr) continue;
			if (chunk->loaded && !chunk->permanentlyLoaded && current - chunk->lastRendered > 30) {
				delete chunks[ch];
				chunks.erase(ch);
				goto unloader_func_chunks_loop;
			}
		}
		chunksMutex.unlock();
	}
}

void World::loadChunk(glm::ivec2 pos, bool permanentlyLoaded) {
	std::size_t chunkCh = hashPos(pos);

	if (chunks.find(chunkCh) == chunks.end()) {
		Chunk* chunk = new Chunk();
		chunk->blocks = std::unordered_map<std::size_t, Block*>();
		for (int i = 0; i < BLOCK_TYPE_COUNT; i++) {
			chunk->renderingGroups[(BLOCK_TYPE)i] = std::vector<Block*>();
		}
		chunk->permanentlyLoaded = permanentlyLoaded;
		chunk->lastRendered = time(nullptr);
		chunksMutex.lock();
		chunks[chunkCh] = chunk;
		chunksMutex.unlock();
	} else {
		return; // already loaded
	}

	chunkLoadQueueMutex.lock();
	chunkLoadQueue.push_back(pos);
	chunkLoadQueueMutex.unlock();
}

size_t World::chunksLoaded() {
	chunksMutex.lock();
	size_t size = chunks.size();
	chunksMutex.unlock();
	return size;
}

size_t World::chunkLoadQueueCount() {
	chunkLoadQueueMutex.lock();
	size_t size = chunkLoadQueue.size();
	chunkLoadQueueMutex.unlock();
	return size;
}
