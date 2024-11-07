#include "World.h"

World::World() {
    for (int x = 0; x < 20; x++) {
        for (int y = 0; y < 5; y++) {
            for (int z = 0; z < 20; z++) {
                setBlock(glm::vec3(x - 10, y, z - 10), y == 4 ? BLOCK_TYPE::GRASS : y == 3 ? BLOCK_TYPE::DIRT : BLOCK_TYPE::STONE);
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
    if (type == BLOCK_TYPE::AIR) return;

    blocks[ch] = new Block(type, pos);
}
