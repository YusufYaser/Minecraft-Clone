#include "World.h"

World::World(siv::PerlinNoise::seed_type seed, glm::vec2 size) {
    const siv::PerlinNoise perlin{ seed };

    for (int x = 0; x < size.x; x++) {
        for (int z = 0; z < size.y; z++) {
            const double random = perlin.octave2D_01((x * 0.025), (z * 0.025), 4);
            int height = random * 10;
            for (int y = 0; y < height; y++) {
                setBlock(glm::vec3(x - size.x/2, y, z - size.y/2), y == height - 1 ? BLOCK_TYPE::GRASS : y == height - 2 ? BLOCK_TYPE::DIRT : BLOCK_TYPE::STONE);
            }
        }
    }
}

World::~World()
{
    for (auto& block : blocks) {
        block.second->~Block();
    }
}

void World::Render(GLuint shader)
{
    for (auto& block : blocks) {
        block.second->Render(shader);
    }
}

Block* World::getBlock(glm::vec3 pos)
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

void World::setBlock(glm::vec3 pos, BLOCK_TYPE type, bool replace)
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
            int opposite = 0;
            switch ((BLOCK_FACE)i) {
            case BLOCK_FACE::TOP:
                opposite = (int)BLOCK_FACE::BOTTOM;
                break;
            case BLOCK_FACE::BOTTOM:
                opposite = (int)BLOCK_FACE::TOP;
                break;
            case BLOCK_FACE::RIGHT:
                opposite = (int)BLOCK_FACE::LEFT;
                break;
            case BLOCK_FACE::LEFT:
                opposite = (int)BLOCK_FACE::RIGHT;
                break;
            case BLOCK_FACE::FRONT:
                opposite = (int)BLOCK_FACE::BACK;
                break;
            case BLOCK_FACE::BACK:
                opposite = (int)BLOCK_FACE::FRONT;
                break;
            }
            otherBlock->hiddenFaces -= 1 << opposite;
            otherBlock->updateVertices();
        }
        return;
    }

    uint8_t hiddenFaces = 0;

    for (int i = 0; i < 6; i++) {
        Block* otherBlock = getBlock(pos + getBlockFaceDirection((BLOCK_FACE)i));
        if (otherBlock == nullptr) continue;
        int opposite = 0;
        switch ((BLOCK_FACE)i) {
        case BLOCK_FACE::TOP:
            opposite = (int)BLOCK_FACE::BOTTOM;
            break;
        case BLOCK_FACE::BOTTOM:
            opposite = (int)BLOCK_FACE::TOP;
            break;
        case BLOCK_FACE::RIGHT:
            opposite = (int)BLOCK_FACE::LEFT;
            break;
        case BLOCK_FACE::LEFT:
            opposite = (int)BLOCK_FACE::RIGHT;
            break;
        case BLOCK_FACE::FRONT:
            opposite = (int)BLOCK_FACE::BACK;
            break;
        case BLOCK_FACE::BACK:
            opposite = (int)BLOCK_FACE::FRONT;
            break;
        }
        otherBlock->hiddenFaces += 1 << opposite;
        otherBlock->updateVertices();
        hiddenFaces += 1 << i;
    }

    blocks[ch] = new Block(type, pos, hiddenFaces);
}
