#include "World.h"
#include "Utils.h"
#include "../Game/Game.h"

World::World(WorldSettings& settings) {
    World::seed = settings.seed;
    World::perlin = siv::PerlinNoise{ seed };
    World::generator = settings.generator;

    int structureCount = 0;
    for (STRUCTURE_TYPE structureType : settings.structures) {
        Structure* structure = Structure::getStructure(structureType);
        structures.push_back(structure);
    }

    std::sort(structures.begin(), structures.end(), [](Structure* a, Structure* b) {
        return a->getPriority() > b->getPriority();
        });

    unloading.store(false);

    chunkLoader = std::thread([this]() {
        chunkLoaderFunc();
        });

    chunkUnloader = std::thread([this]() {
        chunkUnloaderFunc();
        });
}

World::~World()
{
    unloading.store(true);
    print("Waiting for chunk loader thread...");
    chunkLoader.join();
    print("Waiting for chunk unloader thread...");
    chunkUnloader.join();

    print("Removing chunks");
    for (auto& [ch, chunk] : chunks) {
        // TODO: use chunk unloader instead
        delete chunk;
    }
}

void World::render(Shader* shader) {
    Player* player = Game::getInstance()->getPlayer();
    if (player == nullptr) return;

    shader->activate();
    glUniformMatrix4fv(shader->getUniformLoc("view"), 1, GL_FALSE, glm::value_ptr(player->getView()));
    glUniformMatrix4fv(shader->getUniformLoc("projection"), 1, GL_FALSE, glm::value_ptr(player->getProjection()));

    Block* targetBlock = nullptr;
    player->getTargetBlock(&targetBlock);

    if (targetBlock != nullptr) targetBlock->highlighted = true;

    glm::vec3 pos = player->getCameraPos();
    int renderDistance = Game::getInstance()->getRenderDistance();
    glUniform1i(shader->getUniformLoc("highlighted"), 0);

    const int CHUNKS_TO_LOAD_SIZE = 256;
    glm::ivec2 chunksToLoad[CHUNKS_TO_LOAD_SIZE];
    int chunksToLoadc = 0;

    float cosHalfFOV = std::cos(glm::radians(190.0f / 2.0f));

    glm::ivec2 playerChunk = getPosChunk(player->pos);

    for (int x = -(renderDistance / 2) + (pos.x / 16); x < (renderDistance / 2) + (pos.x / 16); x++) {
        for (int y = -(renderDistance / 2) + (pos.z / 16); y < (renderDistance / 2) + (pos.z / 16); y++) {
            glm::ivec2 cPos = glm::ivec2(x, y);

            if (glm::length(glm::vec2(cPos - playerChunk)) > 1) {
                glm::vec3 chunkCenter = glm::vec3(x * 16 + 8, 0.0f, y * 16 + 8);    
                glm::vec3 playerPos3D = pos;

                glm::vec3 toChunk = glm::normalize(chunkCenter - playerPos3D);
                glm::vec3 viewDir = glm::normalize(player->orientation);

                if (glm::dot(toChunk, viewDir) < cosHalfFOV) continue;
            }

            std::size_t chunkCh = hashPos(cPos);
            chunksMutex.lock();
            std::unordered_map<std::size_t, Chunk*>::iterator it = chunks.find(chunkCh);
            if (it == chunks.end())
            {
                chunksMutex.unlock();
                if (chunksToLoadc >= CHUNKS_TO_LOAD_SIZE) continue;
                chunksToLoad[chunksToLoadc++] = cPos;
                continue;
            }
            Chunk* chunk = it->second;
            chunksMutex.unlock();
            if (chunk == nullptr) continue;
            if (!chunk->loaded) continue;

            if (!chunk->renderingGroupsMutex.try_lock()) continue;
            std::vector<BLOCK_TYPE> queued;
            for (auto& [type, blocks] : chunk->renderingGroups) {
                if (isBlockTypeTransparent(type)) { // TODO: do something better than this
                    queued.push_back(type);
                    continue;
                }
                glBindTexture(GL_TEXTURE_2D, getTexture(getTextureName(type)));

                for (auto& block : blocks) {
                    if (block == nullptr) continue;
                    block->Render(shader, false);
                }
            }
            for (auto& type : queued) {
                auto& blocks = chunk->renderingGroups[type];
                glBindTexture(GL_TEXTURE_2D, getTexture(getTextureName(type)));

                for (auto& block : blocks) {
                    if (block == nullptr) continue;
                    block->Render(shader, false);
                }
            }
            chunk->lastRendered = time(nullptr);
            chunk->renderingGroupsMutex.unlock();
        }
    }

    if (chunksToLoadc < 2) { // sorting doesn't really matter in that case
        for (int i = 0; i < chunksToLoadc; i++) {
            loadChunk(chunksToLoad[i]);
        }
    }
    else {
        auto sorter = [&pos](glm::ivec2 a, glm::ivec2 b) {
            glm::ivec2 pos2 = { pos.x, pos.z };
            glm::ivec2 aDiff = a - pos2;
            int aDist = aDiff.x * aDiff.x + aDiff.y * aDiff.y;

            glm::ivec2 bDiff = b - pos2;
            int bDist = bDiff.x * bDiff.x + bDiff.y * bDiff.y;

            return aDist < bDist;
            };

        std::sort(chunksToLoad, chunksToLoad + chunksToLoadc, sorter);

        for (int i = 0; i < chunksToLoadc; i++) {
            loadChunk(chunksToLoad[i]);
        }
    }

    if (targetBlock != nullptr) targetBlock->highlighted = false;
}

Block* World::getBlock(glm::ivec3 pos) {
    std::size_t blockCh = hashPos(pos);
    std::size_t chunkCh = hashPos(getPosChunk(pos));

    chunksMutex.lock();
    if (chunks.find(chunkCh) == chunks.end())
    {
        chunksMutex.unlock();
        return nullptr; // chunk not loaded
    }

    Chunk* chunk = chunks[chunkCh];
    chunksMutex.unlock();
    if (chunk == nullptr) return nullptr;

    chunk->blocksMutex.lock();
    if (chunk->blocks.find(blockCh) != chunk->blocks.end()) {
        chunk->blocksMutex.unlock();
        
        return chunk->blocks[blockCh];
    }
    chunk->blocksMutex.unlock();

    return nullptr;
}

Block* World::setBlock(glm::ivec3 pos, BLOCK_TYPE type, bool replace) {
    if (type == BLOCK_TYPE::NONE) return nullptr;

    std::size_t blockCh = hashPos(pos);
    std::size_t chunkCh = hashPos(getPosChunk(pos));

    chunksMutex.lock();
    if (chunks.find(chunkCh) == chunks.end())
    {
        chunksMutex.unlock();
        return nullptr; // chunk not loaded
    }

    Chunk* chunk = chunks[chunkCh];
    chunksMutex.unlock();
    if (chunk == nullptr) return nullptr;

    chunk->blocksMutex.lock();
    if (chunk->blocks.find(blockCh) != chunk->blocks.end())
    {
        chunk->blocksMutex.unlock();
        if (!replace) return nullptr;

        Block* oldBlock = chunk->blocks[blockCh];
        chunk->renderingGroupsMutex.lock();
        std::vector<Block*>* renderingGroup = &(chunk->renderingGroups[oldBlock->getType()]);
        std::vector<Block*>::iterator begin = renderingGroup->begin();
        std::vector<Block*>::iterator end = renderingGroup->end();
        std::vector<Block*>::iterator it = std::find(begin, end, chunk->blocks[blockCh]);
        if (it != end)
        {
            renderingGroup->erase(it);
        }
        chunk->renderingGroupsMutex.unlock();

        chunk->blocksMutex.lock();
        delete chunk->blocks[blockCh];
        chunk->blocks.erase(blockCh);
        chunk->blocksMutex.unlock();
    }
    else {
        chunk->blocksMutex.unlock();
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
            setRenderingGroup(otherBlock);
        }
        return nullptr;
    }

    uint8_t hiddenFaces = 0;

    Block* block = new Block(type, pos);

    for (int i = 0; i < 6; i++) {
        Block* otherBlock = getBlock(pos + getBlockFaceDirection((BLOCK_FACE)i));
        if (otherBlock == nullptr) continue;
        int opposite = (i % 2 == 0) ? i + 1 : i - 1;
        if (block->hasTransparency() != otherBlock->hasTransparency()) {
            if ((otherBlock->hiddenFaces & (1 << opposite)) != 0) {
                otherBlock->hiddenFaces ^= 1 << opposite;
            }
            continue;
        }
        otherBlock->hiddenFaces |= 1 << opposite;
        hiddenFaces |= 1 << i;
        setRenderingGroup(otherBlock);
    }

    block->hiddenFaces = hiddenFaces;

    chunk->blocksMutex.lock();
    chunk->blocks[blockCh] = block;
    chunk->blocksMutex.unlock();

    setRenderingGroup(block);

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
        if (it != end)
        {
            renderingGroup->erase(it);
        }
        chunk->renderingGroupsMutex.unlock();
        return;
    }

    begin = renderingGroup->begin();
    end = renderingGroup->end();
    it = std::find(begin, end, block);
    if (it != end)
    {
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
