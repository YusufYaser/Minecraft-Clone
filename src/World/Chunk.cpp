#include "World.h"
#include "Utils.h"

void World::chunkLoaderFunc()
{
    Structure* structures[STRUCTURES_COUNT];
    for (int i = 0; i < STRUCTURES_COUNT; i++) {
        structures[i] = Structure::getStructure(STRUCTURE_TYPE(i));
    }

    std::sort(structures, structures + STRUCTURES_COUNT, [](Structure* a, Structure* b) {
        return a->getPriority() > b->getPriority();
    });

    while (true) {
        if (unloading.load()) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));

        if (chunkLoadQueue.size() == 0) continue;
        chunkLoadQueueMutex.lock();
        glm::ivec2 pos = chunkLoadQueue.front();
        chunkLoadQueue.erase(chunkLoadQueue.begin());
        chunkLoadQueueMutex.unlock();

        std::size_t chunkCh = hashPos(pos);
        Chunk* chunk = chunks[chunkCh];

        for (int x = pos.x * 16; x < 16 + pos.x * 16; x++) {
            for (int z = pos.y * 16; z < 16 + pos.y * 16; z++) {
                if (unloading.load()) break;

                int height = getHeight({ x, z });
                for (int y = 0; y < height; y++) {
                    if (unloading.load()) break;
                    BLOCK_TYPE type = BLOCK_TYPE::STONE;
                    if (y == height - 1) type = BLOCK_TYPE::GRASS;
                    else if (y >= height - 3) type = BLOCK_TYPE::DIRT;

                    setBlock(glm::ivec3(x, y, z), type);
                }

                int MAX_STRUCTURE_HEIGHT = 0;

                int bases[STRUCTURES_COUNT]{};
                Structure* structuresInChunk[STRUCTURES_COUNT]{};
                int structuresInXYCount = 0;

                for (int i = 0; i < STRUCTURES_COUNT; i++) {
                    Structure* structure = structures[i];
                    if (!structure->isInXZ({ x, z })) continue;

                    int height = structure->getHeight();
                    height -= structure->getPivot().y;
                    if (height > MAX_STRUCTURE_HEIGHT) MAX_STRUCTURE_HEIGHT = height;

                    int base = structure->getBase({ x, z });
                    bases[structuresInXYCount] = base;
                    structuresInChunk[structuresInXYCount++] = structure;
                }
                
                if (structuresInXYCount == 0) continue;

                for (int y = 0; y < height + MAX_STRUCTURE_HEIGHT + 15; y++) {
                    for (int i = 0; i < structuresInXYCount; i++) {
                        Structure* structure = structuresInChunk[i];
                        int base = bases[i];

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

void World::chunkUnloaderFunc()
{
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

void World::loadChunk(glm::ivec2 pos, bool permanentlyLoaded)
{
    std::size_t chunkCh = hashPos(pos);

    if (chunks.find(chunkCh) == chunks.end())
    {
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
    }
    else {
        return; // already loaded
    }

    chunkLoadQueueMutex.lock();
    chunkLoadQueue.push_back(pos);
    chunkLoadQueueMutex.unlock();
}

int World::chunksLoaded()
{
    chunksMutex.lock();
    int size = chunks.size();
    chunksMutex.unlock();
    return size;
}

int World::chunkLoadQueueCount()
{
    chunkLoadQueueMutex.lock();
    int size = chunkLoadQueue.size();
    chunkLoadQueueMutex.unlock();
    return size;
}
