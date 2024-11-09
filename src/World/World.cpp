#include "World.h"

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

World::World(siv::PerlinNoise::seed_type seed, float* progress, glm::ivec2 size) {
    const int THREAD_COUNT = 8;

    World::seed = seed;

    const siv::PerlinNoise perlin{ seed };

    auto generate = [&perlin, &size, this](int startX, int endX, int startZ, int endZ) {
        for (int x = startX; x < endX; x++) {
            for (int z = startZ; z < endZ; z++) {
                const double random = perlin.octave2D_01((x * 0.025), (z * 0.025), 4);
                int height = round(random * 30 + 2);
                for (int y = 0; y < height; y++) {
                    BLOCK_TYPE type = BLOCK_TYPE::STONE;
                    if (y == height - 1) type = BLOCK_TYPE::GRASS;
                    else if (y >= height - 3) type = BLOCK_TYPE::DIRT;
                    setBlock(glm::ivec3(x - size.x / 2, y, z - size.y / 2), type);
                }
            }
        }
    };

    std::thread threads[THREAD_COUNT];
    for (int i = 0; i < THREAD_COUNT; i++) {
        threads[i] = std::thread(generate, 0, size.x*i / THREAD_COUNT, 0, size.y);
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        while (!threads[i].joinable()) {}
        threads[i].join();
    }

    for (auto& [ch, block] : blocks) {
        setRenderingGroup(block);
    }
    loading = false;
}

World::~World()
{
    for (auto& [ch, block] : blocks) {
        delete block;
    }
}

void World::Render(GLuint shader, glm::vec3 pos, float renderDistance)
{
    glUseProgram(shader);
    glUniform3fv(glGetUniformLocation(shader, "highlightColor"), 1, glm::value_ptr(glm::vec3(1.0f)));

    for (auto& [type, blocks] : renderingGroups) {
        glBindTexture(GL_TEXTURE_2D, getTexture(getTextureName(type)));

        for (auto& block : blocks) {
            glm::ivec3 blockPos = block->getPos();
            int x = pos.x - blockPos.x;
            int z = pos.z - blockPos.z;
            float distanceSquared = x * x + z * z;

            if (distanceSquared > renderDistance*renderDistance) continue;

            block->Render(shader, false);
        }
    }
}

Block* World::getBlock(glm::ivec3 pos)
{
    std::size_t ch = hashPos(pos);
    if (blocks.find(ch) != blocks.end())
    {
        return blocks[ch];
    }

    return nullptr;
}

void World::setBlock(glm::ivec3 pos, BLOCK_TYPE type, bool replace)
{
    std::size_t ch = hashPos(pos);
    std::lock_guard<std::mutex> guard(blocksMutex);
    if (blocks.find(ch) != blocks.end())
    {
        if (loading) return;
        if (!replace) return;
        delete blocks[ch];
        blocks.erase(ch);
    }
    if (type == BLOCK_TYPE::AIR) {
        for (int i = 0; i < 6; i++) {
            Block* otherBlock = getBlock(pos + getBlockFaceDirection((BLOCK_FACE)i));
            if (otherBlock == nullptr) continue;
            int opposite = (i % 2 == 0) ? i + 1 : i - 1;
            otherBlock->hiddenFaces ^= 1 << opposite;
            if (!loading) setRenderingGroup(otherBlock);
        }
        return;
    }

    uint8_t hiddenFaces = 0;

    for (int i = 0; i < 6; i++) {
        Block* otherBlock = getBlock(pos + getBlockFaceDirection((BLOCK_FACE)i));
        if (otherBlock == nullptr) continue;
        int opposite = (i % 2 == 0) ? i + 1 : i - 1;
        otherBlock->hiddenFaces |= 1 << opposite;
        hiddenFaces |= 1 << i;
        if (!loading) setRenderingGroup(otherBlock);
    }

    Block* block = new Block(type, pos, hiddenFaces);
    blocks[ch] = block;

    if (!loading) setRenderingGroup(block);
}

void World::setRenderingGroup(Block* block)
{
    BLOCK_TYPE type = block->getType();
    if (renderingGroups.find(type) == renderingGroups.end())
    {
        renderingGroups[type] = std::vector<Block*>();
    }
    if (block == NULL || block->hiddenFaces == 63) {
        if (loading) return;
        std::vector<Block*>::iterator begin = renderingGroups[type].begin();
        std::vector<Block*>::iterator end = renderingGroups[type].end();
        std::vector<Block*>::iterator it = std::find(begin, end, block);
        if (it != renderingGroups[type].end())
        {
            renderingGroups[type].erase(it);
        }
        return;
    }
    if (!loading) {
        std::vector<Block*>::iterator begin = renderingGroups[type].begin();
        std::vector<Block*>::iterator end = renderingGroups[type].end();
        std::vector<Block*>::iterator it = std::find(begin, end, block);
        if (it != renderingGroups[type].end())
        {
            return;
        }
    }
    glm::ivec3 pos = block->getPos();
    std::size_t ch = hashPos(pos);

    renderingGroups[type].push_back(block);
}
