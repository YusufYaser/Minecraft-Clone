#include "World.h"
#include "Utils.h"

void World::chunkLoaderFunc()
{
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

                if (random({ x, z }) < .0025) {
                    glm::ivec3 base = glm::ivec3(x, height, z);
                    int treeHeight = 4;
                    for (int i = 0; i < treeHeight; i++) {
                        setBlock(base + glm::ivec3(0, i, 0), BLOCK_TYPE::OAK_LOG);
                    }
                    for (int i = treeHeight - 1; i < treeHeight + 2; i++) {
                        int s = 2;
                        if (i == treeHeight + 1) s = 1;
                        for (int tx = -s; tx <= s; tx++) {
                            for (int tz = -s; tz <= s; tz++) {
                                if (i == treeHeight - 1 && tx == 0 && tz == 0) continue;
                                setBlock(base + glm::ivec3(tx, i, tz), BLOCK_TYPE::OAK_LEAVES);
                            }
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
