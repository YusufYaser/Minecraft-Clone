#include "World.h"
#include "Utils.h"

Block* World::getBlock(glm::ivec3 pos) {
	if (pos.y >= MAX_HEIGHT || pos.y < 0) return nullptr;

	std::size_t chunkCh = hashPos(getPosChunk(pos));

	chunksMutex.lock();
	Chunk* chunk = chunks[chunkCh];
	chunksMutex.unlock();
	if (chunk == nullptr) return nullptr;
	if (chunk->maxHeight < pos.y) return nullptr;

	chunk->blocksMutex.lock();
	Block* block = chunk->blocks[hashPosForChunk(pos)];
	chunk->blocksMutex.unlock();

	return block;
}

void World::fillBlocks(glm::ivec3 start, glm::ivec3 end, BLOCK_TYPE type) {
	if (type == BLOCK_TYPE::NONE || (uint8_t)type >= BLOCK_TYPE_COUNT) return;

	start.y = std::min(MAX_HEIGHT, std::max(0, start.y));
	end.y = std::min(MAX_HEIGHT, std::max(0, end.y));

	std::unordered_map<Chunk*, bool> cLoaded;

	Chunk* chunk = nullptr;
	std::size_t chunkCh = 0;

	for (int x = start.x + 1; x <= end.x - 1; x++) {
		for (int y = start.y + 1; y <= end.y - 1; y++) {
			for (int z = start.z + 1; z <= end.z - 1; z++) {
				glm::ivec3 pos = { x, y, z };
				std::size_t ncch = hashPos(getPosChunk(pos));
				if (chunkCh != ncch) {
					chunksMutex.lock();
					auto cit = chunks.find(ncch);
					chunksMutex.unlock();
					if (cit == chunks.end()) {
						continue; // chunk not loaded
					}

					chunk = cit->second;
					chunkCh = ncch;
					if (!cLoaded[chunk]) {
						cLoaded[chunk] = true;
						chunk->blocksMutex.lock();
						chunk->setMaxHeight(end.y);
					}

					chunk->modified = true;
					if (chunk->loaded) m_worldRenderModified = true;
				}
				std::size_t blockCh = hashPos(pos);

				Block* block = new Block(type, pos);
				block->hiddenFaces = 63;
#ifdef GAME_DEBUG
				block->dPlacementMethod = BLOCK_PLACEMENT_METHOD::FILL_BLOCKS;
#endif
				chunk->blocks[hashPosForChunk(pos)] = block;
			}
		}
	}

	for (auto& [chunk, _] : cLoaded) {
		chunk->blocksMutex.unlock();
	}

	for (int y = start.y; y <= end.y; y++) {
		for (int z = start.z; z <= end.z; z++) {
			setBlock({ start.x, y, z }, type, true);
			setBlock({ end.x, y, z }, type, true);
		}
	}

	for (int x = start.x; x <= end.x; x++) {
		for (int z = start.z; z <= end.z; z++) {
			setBlock({ x, start.y, z }, type, true);
			setBlock({ x, end.y, z }, type, true);
		}
	}

	for (int x = start.x; x <= end.x; x++) {
		for (int y = start.y; y <= end.y; y++) {
			setBlock({ x, y, start.z }, type, true);
			setBlock({ x, y, end.z }, type, true);
		}
	}
}

Block* World::setBlock(glm::ivec3 pos, BLOCK_TYPE type, bool replace) {
	if (type == BLOCK_TYPE::NONE || (uint8_t)type >= BLOCK_TYPE_COUNT) return nullptr;

	if (pos.y < 0 || pos.y >= MAX_HEIGHT) return nullptr;
	if (unloading.load()) return nullptr;

	std::size_t blockCh = hashPos(pos);
	std::size_t chunkCh = hashPos(getPosChunk(pos));

	chunksMutex.lock();
	auto cit = chunks.find(chunkCh);
	chunksMutex.unlock();
	if (cit == chunks.end()) {
		return nullptr; // chunk not loaded
	}

	Chunk* chunk = cit->second;

	chunk->blocksMutex.lock();
	chunk->setMaxHeight(pos.y);
	Block* block = chunk->blocks[hashPosForChunk(pos)];
	chunk->blocksMutex.unlock();
	if (block != nullptr) {
		if (!replace) return nullptr;

		Block* oldBlock = block;
		chunk->renderingGroupsMutex.lock();
		std::vector<Block*>* renderingGroup = &(chunk->blocksToRender);
		std::vector<Block*>::iterator begin = renderingGroup->begin();
		std::vector<Block*>::iterator end = renderingGroup->end();
		std::vector<Block*>::iterator it = std::find(begin, end, chunk->blocks[hashPosForChunk(pos)]);
		if (it != end) {
			renderingGroup->erase(it);
		}
		chunk->renderingGroupsMutex.unlock();

		delete oldBlock;
		chunk->blocksMutex.lock();
		chunk->blocks[hashPosForChunk(pos)] = nullptr;
		chunk->blocksMutex.unlock();
		chunk->modified = true;
		if (chunk->loaded) m_worldRenderModified = true;
	} else {
		if (type == BLOCK_TYPE::AIR) return nullptr; // it was already air, nothing to change
	}

	if (type == BLOCK_TYPE::AIR) {
		for (int i = 0; i < 6; i++) {
			Block* otherBlock = getBlock(pos + faceDirections[i]);
			if (otherBlock == nullptr) continue;
			int opposite = (i % 2 == 0) ? i + 1 : i - 1;
			if ((otherBlock->hiddenFaces & (1 << opposite)) != 0) {
				otherBlock->hiddenFaces ^= 1 << opposite;
			}
			setRenderingGroup(otherBlock);
		}
		return nullptr;
	}

	uint8_t hiddenFaces = 0;

	block = new Block(type, pos);

	for (int i = 0; i < 6; i++) {
		Block* otherBlock = getBlock(pos + faceDirections[i]);
		if (otherBlock == nullptr) continue;
		int opposite = (i % 2 == 0) ? i + 1 : i - 1;
		if (block->hasTransparency() != otherBlock->hasTransparency()) {
			if (block->hasTransparency()) {
				hiddenFaces |= 1 << i;
			} else if (otherBlock->type != BLOCK_TYPE::WATER || opposite != int(BLOCK_FACE::TOP)) {
				if ((otherBlock->hiddenFaces & (1 << opposite)) == 0) {
					otherBlock->hiddenFaces ^= 1 << opposite;
				}
			}
		} else {
			otherBlock->hiddenFaces |= 1 << opposite;
			hiddenFaces |= 1 << i;
		}
		setRenderingGroup(otherBlock);
	}

	block->hiddenFaces = hiddenFaces;
#ifdef GAME_DEBUG
	block->dPlacementMethod = BLOCK_PLACEMENT_METHOD::SET_BLOCK;
#endif

	chunk->blocksMutex.lock();
	chunk->blocks[hashPosForChunk(pos)] = block;
	chunk->blocksMutex.unlock();

	setRenderingGroup(block);

	chunk->modified = true;
	if (chunk->loaded) m_worldRenderModified = true;
	return block;
}

void World::setRenderingGroup(Block* block) {
	std::size_t blockCh = hashPos(block->pos);
	std::size_t chunkCh = hashPos(getPosChunk(block->pos));

	chunksMutex.lock();
	auto cit = chunks.find(chunkCh);
	chunksMutex.unlock();
	if (cit == chunks.end()) {
		return; // chunk not loaded
	}

	Chunk* chunk = cit->second;

	BLOCK_TYPE type = block->type;
	chunk->renderingGroupsMutex.lock();
	if (block == nullptr || block->hiddenFaces == 63) {
		std::vector<Block*>::iterator end = chunk->blocksToRender.end();
		std::vector<Block*>::iterator it = std::find(chunk->blocksToRender.begin(), end, block);
		if (it != end) {
			chunk->blocksToRender.erase(it);
		}
		chunk->renderingGroupsMutex.unlock();
		return;
	}

	std::vector<Block*>::iterator end = chunk->blocksToRender.end();
	std::vector<Block*>::iterator it = std::find(chunk->blocksToRender.begin(), end, block);
	if (it != end) {
		chunk->renderingGroupsMutex.unlock();
		return;
	}
	chunk->renderingGroupsMutex.unlock();

	glm::ivec3 pos = block->pos;
	std::size_t ch = hashPos(pos);

	chunk->renderingGroupsMutex.lock();
	chunk->blocksToRender.push_back(block);
	chunk->renderingGroupsMutex.unlock();
}
