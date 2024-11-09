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

World::World(siv::PerlinNoise::seed_type seed, glm::ivec2 size) {
    World::seed = seed;

    const siv::PerlinNoise perlin{ seed };

    for (int x = -size.x/2; x < size.x/2; x++) {
        for (int z = -size.y/2; z < size.y/2; z++) {
            const double random = perlin.octave2D_01((x * 0.025), (z * 0.025), 4);
            int height = round(random * 30 + 2);
            for (int y = 0; y < height; y++) {
                BLOCK_TYPE type = BLOCK_TYPE::STONE;
                if (y == height - 1) type = BLOCK_TYPE::GRASS;
                else if (y >= height - 3) type = BLOCK_TYPE::DIRT;
                setBlock(glm::ivec3(x, y, z), type);
            }
        }
    }

    loading = false;
    setRenderingGroups();
}

World::~World()
{
    for (auto& block : blocks) {
        block.second->~Block();
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
            int y = pos.y - blockPos.y;
            int z = pos.y - blockPos.z;
            float distanceSquared = x * x + y * y + z * z;

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
    if (blocks.find(ch) != blocks.end())
    {
        if (loading) {
            print("found block at same coords while loading world", pos.x, pos.y, pos.z);
            return;
        }
        if (!replace) return;
        blocks[ch]->~Block();
        blocks.erase(ch);
    }
    if (type == BLOCK_TYPE::AIR) {
        for (int i = 0; i < 6; i++) {
            Block* otherBlock = getBlock(pos + getBlockFaceDirection((BLOCK_FACE)i));
            if (otherBlock == nullptr) continue;
            int opposite = (i % 2 == 0) ? i + 1 : i - 1;
            otherBlock->hiddenFaces ^= 1 << opposite;
        }
        setRenderingGroups();
        return;
    }

    uint8_t hiddenFaces = 0;

    for (int i = 0; i < 6; i++) {
        Block* otherBlock = getBlock(pos + getBlockFaceDirection((BLOCK_FACE)i));
        if (otherBlock == nullptr) continue;
        int opposite = (i % 2 == 0) ? i + 1 : i - 1;
        otherBlock->hiddenFaces |= 1 << opposite;
        hiddenFaces |= 1 << i;
    }

    Block* block = new Block(type, pos, hiddenFaces);
    blocks[ch] = block;

    if (!loading) setRenderingGroups();
}

void World::setRenderingGroups()
{
    for (auto& block : blocks) {
        if (block.second == NULL || block.second->hiddenFaces == 63) continue;
        glm::ivec3 pos = block.second->getPos();
        std::size_t ch = hashPos(pos);

        BLOCK_TYPE type = block.second->getType();

        if (renderingGroups.find(type) == renderingGroups.end())
        {
            renderingGroups[type] = std::vector<Block*>();
        }
        renderingGroups[type].push_back(block.second);
    }
}
