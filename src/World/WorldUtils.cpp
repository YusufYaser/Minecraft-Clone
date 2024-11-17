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

int World::getHeight(glm::ivec2 pos)
{
    const double random = perlin.octave2D_01((pos.x * 0.025), (pos.y * 0.025), 4);
    int height = round(random * 10 + 2);
    return height;
}

double World::random(glm::ivec2 pos)
{
    std::uniform_real_distribution<> dist(0.0, 1.0);
    std::uint32_t hash = seed;
    hash ^= pos.x + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    hash ^= pos.y + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    std::mt19937 rng(hash);
    return dist(rng);
}
