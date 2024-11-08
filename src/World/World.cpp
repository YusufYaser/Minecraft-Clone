#include "World.h"

World::World(siv::PerlinNoise::seed_type seed, glm::vec2 size) {
    World::seed = seed;

    const siv::PerlinNoise perlin{ seed };

    for (int x = 0; x < size.x; x++) {
        for (int z = 0; z < size.y; z++) {
            const double random = perlin.octave2D_01((x * 0.025), (z * 0.025), 4);
            int height = random * 10 + 2;
            for (int y = 0; y < height; y++) {
                BLOCK_TYPE type = BLOCK_TYPE::STONE;
                if (y == height - 1) type = BLOCK_TYPE::GRASS;
                else if (y >= height - 3) type = BLOCK_TYPE::DIRT;
                setBlock(glm::vec3(x - size.x/2, y, z - size.y/2), type);
            }
        }
    }

    loading = false;
    setBlocksToRender();
}

World::~World()
{
    for (auto& block : blocks) {
        block.second->~Block();
    }
}

void World::Render(GLuint shader)
{
    glUseProgram(shader);
    glUniform3fv(glGetUniformLocation(shader, "highlightColor"), 1, glm::value_ptr(glm::vec3(1.0f)));

    for (auto& type : renderingGroups) {
        glBindTexture(GL_TEXTURE_2D, getTexture(getTextureName(type.first)));

        for (auto& ch : type.second) {
            blocks[ch]->Render(shader, false);
        }
    }
}

Block* World::getBlock(glm::ivec3 pos)
{
    int x = pos.x;
    int y = pos.y;
    int z = pos.z;
    int ch = std::hash<int>()(x) ^ (std::hash<int>()(y) << 1) ^ (std::hash<int>()(z) << 2);
    if (blocks.find(ch) != blocks.end())
    {
        return blocks[ch];
    }

    return nullptr;
}

void World::setBlock(glm::ivec3 pos, BLOCK_TYPE type, bool replace)
{
    int x = pos.x;
    int y = pos.y;
    int z = pos.z;
    int ch = std::hash<int>()(x) ^ (std::hash<int>()(y) << 1) ^ (std::hash<int>()(z) << 2);
    if (blocks.find(ch) != blocks.end())
    {
        if (!replace) return;
        blocks[ch]->~Block();
        blocks.erase(ch);
    }
    if (type == BLOCK_TYPE::AIR) {
        for (int i = 0; i < 6; i++) {
            Block* otherBlock = getBlock(pos + getBlockFaceDirection((BLOCK_FACE)i));
            if (otherBlock == nullptr) continue;
            int opposite = i % 2 == 0 ? i + 1 : i - 1;
            otherBlock->hiddenFaces -= 1 << opposite;
            otherBlock->updateVertices();
        }
        setBlocksToRender();
        return;
    }

    uint8_t hiddenFaces = 0;

    for (int i = 0; i < 6; i++) {
        Block* otherBlock = getBlock(pos + getBlockFaceDirection((BLOCK_FACE)i));
        if (otherBlock == nullptr) continue;
        int opposite = i % 2 == 0 ? i + 1 : i - 1;
        otherBlock->hiddenFaces += 1 << opposite;
        otherBlock->updateVertices();
        hiddenFaces += 1 << i;
    }

    Block* block = new Block(type, pos, hiddenFaces);
    blocks[ch] = block;

    if (!loading) setBlocksToRender();
}

siv::PerlinNoise::seed_type World::getSeed()
{
    return seed;
}

void World::setBlocksToRender()
{
    renderingGroups.clear();
    for (auto& block : blocks) {
        if (block.second == NULL || block.second->hiddenFaces == 63) continue;
        int x = block.second->getPos().x;
        int y = block.second->getPos().y;
        int z = block.second->getPos().z;
        int ch = std::hash<int>()(x) ^ (std::hash<int>()(y) << 1) ^ (std::hash<int>()(z) << 2);

        BLOCK_TYPE type = block.second->getType();

        if (renderingGroups.find(type) == renderingGroups.end())
        {
            renderingGroups[type] = std::vector<int>();
        }
        renderingGroups[type].push_back(ch);
    }
}
