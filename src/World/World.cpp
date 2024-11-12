#include "World.h"
#include "Utils.h"

World::World(siv::PerlinNoise::seed_type seed, glm::ivec2 size) {
    World::seed = seed;

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

void World::Render(GLuint shader, glm::vec3 pos, int renderDistance)
{
    glUseProgram(shader);
    glUniform3fv(glGetUniformLocation(shader, "highlightColor"), 1, glm::value_ptr(glm::vec3(1.0f)));

    for (int x = -(renderDistance / 2) + (pos.x / 16); x < (renderDistance / 2) + (pos.x / 16); x++) {
        for (int y = -(renderDistance / 2) + (pos.z / 16); y < (renderDistance / 2) + (pos.z / 16); y++) {
            glm::ivec2 cPos = glm::ivec2(x, y);
            std::size_t chunkCh = hashPos(cPos);
            std::unordered_map<std::size_t, Chunk*>::iterator it = chunks.find(chunkCh);
            if (it == chunks.end())
            {
                loadChunk(cPos);
                continue;
            }
            Chunk* chunk = it->second;
            if (!chunk->loaded) continue;

            if (!chunk->renderingGroupsMutex.try_lock()) continue;
            for (auto& [type, blocks] : chunk->renderingGroups) {
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
}

Block* World::getBlock(glm::ivec3 pos)
{
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

    chunk->blocksMutex.lock();
    if (chunk->blocks.find(blockCh) != chunk->blocks.end()) {
        chunk->blocksMutex.unlock();
        
        return chunk->blocks[blockCh];
    }
    chunk->blocksMutex.unlock();

    return nullptr;
}

Block* World::setBlock(glm::ivec3 pos, BLOCK_TYPE type, bool replace)
{
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

    chunk->blocksMutex.lock();
    if (chunk->blocks.find(blockCh) != chunk->blocks.end())
    {
        chunk->blocksMutex.unlock();
        if (!replace) return nullptr;
        delete chunk->blocks[blockCh];
        chunk->blocks.erase(blockCh);
    }
    else {
        chunk->blocksMutex.unlock();
    }

    if (type == BLOCK_TYPE::AIR) {
        for (int i = 0; i < 6; i++) {
            Block* otherBlock = getBlock(pos + getBlockFaceDirection((BLOCK_FACE)i));
            if (otherBlock == nullptr) continue;
            int opposite = (i % 2 == 0) ? i + 1 : i - 1;
            otherBlock->hiddenFaces ^= 1 << opposite;
            setRenderingGroup(otherBlock);
        }
        return nullptr;
    }

    uint8_t hiddenFaces = 0;

    for (int i = 0; i < 6; i++) {
        Block* otherBlock = getBlock(pos + getBlockFaceDirection((BLOCK_FACE)i));
        if (otherBlock == nullptr) continue;
        int opposite = (i % 2 == 0) ? i + 1 : i - 1;
        otherBlock->hiddenFaces |= 1 << opposite;
        hiddenFaces |= 1 << i;
        setRenderingGroup(otherBlock);
    }

    Block* block = new Block(type, pos, hiddenFaces);
    chunk->blocksMutex.lock();
    chunk->blocks[blockCh] = block;
    chunk->blocksMutex.unlock();

    setRenderingGroup(block);

    return block;
}

void World::setRenderingGroup(Block* block)
{
    std::size_t blockCh = hashPos(block->getPos());
    std::size_t chunkCh = hashPos(getPosChunk(block->getPos()));

    chunksMutex.lock();
    if (chunks.find(chunkCh) == chunks.end())
    {
        chunksMutex.unlock();
        return; // chunk not loaded
    }

    Chunk* chunk = chunks[chunkCh];
    chunksMutex.unlock();

    BLOCK_TYPE type = block->getType();
    std::vector<Block*>::iterator begin;
    std::vector<Block*>::iterator end;
    std::vector<Block*>::iterator it;
    chunk->renderingGroupsMutex.lock();
    if (chunk->renderingGroups.find(type) == chunk->renderingGroups.end())
    {
        chunk->renderingGroups[type] = std::vector<Block*>();
    }
    if (block == NULL || block->hiddenFaces == 63) {
        begin = chunk->renderingGroups[type].begin();
        end = chunk->renderingGroups[type].end();
        it = std::find(begin, end, block);
        if (it != end)
        {
            chunk->renderingGroups[type].erase(it);
        }
        chunk->renderingGroupsMutex.unlock();
        return;
    }
    chunk->renderingGroupsMutex.unlock();

    chunk->renderingGroupsMutex.lock();
    begin = chunk->renderingGroups[type].begin();
    end = chunk->renderingGroups[type].end();
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
    chunk->renderingGroups[type].push_back(block);
    chunk->renderingGroupsMutex.unlock();
}
