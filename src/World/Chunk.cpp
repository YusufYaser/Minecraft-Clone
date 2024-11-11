#include "World.h"
#include "Utils.h"

inline std::size_t combineHashes(std::size_t hash1, std::size_t hash2) {
    return hash1 ^ (hash2 + 0x9e3779b9 + (hash1 << 6) + (hash1 >> 2));
}

std::size_t hashPos(const glm::ivec3& pos) {
    std::size_t hx = std::hash<int32_t>()(pos.x);
    std::size_t hy = std::hash<int32_t>()(pos.y);
    std::size_t hz = std::hash<int32_t>()(pos.z);

    std::size_t combinedHash = combineHashes(hx, hy);
    combinedHash = combineHashes(combinedHash, hz);
    return combinedHash;
}

std::size_t hashPos(const glm::ivec2& pos) {
    return hashPos(glm::ivec3(pos.x, 0, pos.y));
}

glm::ivec2 getPosChunk(const glm::ivec3& pos) {
    int chunkX = (pos.x >= 0) ? pos.x / 16 : (pos.x - 16 + 1) / 16;
    int chunkZ = (pos.z >= 0) ? pos.z / 16 : (pos.z - 16 + 1) / 16;
    return glm::ivec2(chunkX, chunkZ);
}

void World::loadChunk(glm::ivec2 pos)
{
    std::size_t chunkCh = hashPos(pos);

    if (chunks.find(chunkCh) == chunks.end())
    {
        Chunk* chunk = new Chunk();
        chunk->blocks = std::unordered_map<std::size_t, Block*>();
        chunksMutex.lock();
        chunks[chunkCh] = chunk;
        chunksMutex.unlock();
    }

    auto generate = [this](glm::ivec2 pos) {
        std::size_t chunkCh = hashPos(pos);
        Chunk* chunk = chunks[chunkCh];

        const siv::PerlinNoise perlin{ seed };
        std::uniform_real_distribution<> dist(0.0, 1.0);

        for (int x = pos.x * 16; x < 16 + pos.x * 16; x++) {
            for (int z = pos.y * 16; z < 16 + pos.y * 16; z++) {
                const double random = perlin.octave2D_01((x * 0.025), (z * 0.025), 4);
                int height = round(random * 30 + 2);
                for (int y = 0; y < height; y++) {
                    BLOCK_TYPE type = BLOCK_TYPE::STONE;
                    if (y == height - 1) type = BLOCK_TYPE::GRASS;
                    else if (y >= height - 3) type = BLOCK_TYPE::DIRT;

                    setBlock(glm::ivec3(x, y, z), type);
                }

                /*std::mt19937 rng(seed * x * z);
                if (dist(rng) < .0025) {
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
                }*/
            }
        }

        chunk->loaded = true;
    };

    std::thread t = std::thread(generate, pos);
    t.detach();
}
