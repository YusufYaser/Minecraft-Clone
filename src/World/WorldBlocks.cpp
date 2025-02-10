#include "World.h"
#include "Utils.h"

Block* World::getBlock(glm::ivec3 pos) {
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
	auto bit = chunk->blocks.find(blockCh);
	chunk->blocksMutex.unlock();
	if (bit != chunk->blocks.end()) {
		return bit->second;
	}

	return nullptr;
}

Block* World::setBlock(glm::ivec3 pos, BLOCK_TYPE type, bool replace, bool fast) {
	if (type == BLOCK_TYPE::NONE) return nullptr;

	if (pos.y < 0 || pos.y >= MAX_HEIGHT) return nullptr;

	std::size_t blockCh = hashPos(pos);
	std::size_t chunkCh = hashPos(getPosChunk(pos));

	chunksMutex.lock();
	auto cit = chunks.find(chunkCh);
	chunksMutex.unlock();
	if (cit == chunks.end()) {
		return nullptr; // chunk not loaded
	}

	Chunk* chunk = cit->second;

	if (!fast) {
		chunk->blocksMutex.lock();
		auto bit = chunk->blocks.find(blockCh);
		chunk->blocksMutex.unlock();
		if (bit != chunk->blocks.end()) {
			if (!replace) return nullptr;

			Block* oldBlock = bit->second;
			chunk->renderingGroupsMutex.lock();
			std::vector<Block*>* renderingGroup = &(chunk->renderingGroups[oldBlock->getType()]);
			std::vector<Block*>::iterator begin = renderingGroup->begin();
			std::vector<Block*>::iterator end = renderingGroup->end();
			std::vector<Block*>::iterator it = std::find(begin, end, chunk->blocks[blockCh]);
			if (it != end) {
				renderingGroup->erase(it);
			}
			chunk->renderingGroupsMutex.unlock();

			delete oldBlock;
			chunk->blocksMutex.lock();
			chunk->blocks.erase(blockCh);
			chunk->blocksMutex.unlock();
			chunk->modified = true;
		} else {
			if (type == BLOCK_TYPE::AIR) return nullptr; // it was already air, nothing to change
		}

		if (type == BLOCK_TYPE::AIR) {
			for (int i = 0; i < 6; i++) {
				Block* otherBlock = getBlock(pos + getBlockFaceDirection((BLOCK_FACE)i));
				if (otherBlock == nullptr) continue;
				int opposite = (i % 2 == 0) ? i + 1 : i - 1;
				if ((otherBlock->hiddenFaces & (1 << opposite)) != 0) {
					otherBlock->hiddenFaces ^= 1 << opposite;
				}
				if (otherBlock->hiddenFaces != 63) setRenderingGroup(otherBlock);
			}
			return nullptr;
		}
	}

	uint8_t hiddenFaces = 0;

	Block* block = new Block(type, pos);

	for (int i = 0; i < 6; i++) {
		Block* otherBlock = getBlock(pos + getBlockFaceDirection((BLOCK_FACE)i));
		if (otherBlock == nullptr) continue;
		int opposite = (i % 2 == 0) ? i + 1 : i - 1;
		if (block->hasTransparency() != otherBlock->hasTransparency()) {
			if (block->hasTransparency()) {
				hiddenFaces |= 1 << i;
			} else if ((otherBlock->hiddenFaces & (1 << opposite)) == 0) {
				otherBlock->hiddenFaces ^= 1 << opposite;
			}
			continue;
		}
		otherBlock->hiddenFaces |= 1 << opposite;
		hiddenFaces |= 1 << i;
		if (otherBlock->hiddenFaces != 63) setRenderingGroup(otherBlock);
	}

	block->hiddenFaces = hiddenFaces;

	chunk->blocksMutex.lock();
	chunk->blocks[blockCh] = block;
	chunk->blocksMutex.unlock();

	if (block->hiddenFaces != 63) setRenderingGroup(block);

	chunk->modified = true;
	return block;
}

void World::setRenderingGroup(Block* block) {
	std::size_t blockCh = hashPos(block->getPos());
	std::size_t chunkCh = hashPos(getPosChunk(block->getPos()));

	chunksMutex.lock();
	Chunk* chunk = chunks[chunkCh];
	chunksMutex.unlock();
	if (chunk == nullptr) return;

	BLOCK_TYPE type = block->getType();
	std::vector<Block*>::iterator begin;
	std::vector<Block*>::iterator end;
	std::vector<Block*>::iterator it;
	chunk->renderingGroupsMutex.lock();
	std::vector<Block*>* renderingGroup = &(chunk->renderingGroups[type]);
	if (block == NULL || block->hiddenFaces == 63) {
		begin = renderingGroup->begin();
		end = renderingGroup->end();
		it = std::find(begin, end, block);
		if (it != end) {
			renderingGroup->erase(it);
		}
		chunk->renderingGroupsMutex.unlock();
		return;
	}

	begin = renderingGroup->begin();
	end = renderingGroup->end();
	it = std::find(begin, end, block);
	if (it != end) {
		chunk->renderingGroupsMutex.unlock();
		return;
	}
	chunk->renderingGroupsMutex.unlock();

	glm::ivec3 pos = block->getPos();
	std::size_t ch = hashPos(pos);

	chunk->renderingGroupsMutex.lock();
	renderingGroup->push_back(block);
	chunk->renderingGroupsMutex.unlock();
}
