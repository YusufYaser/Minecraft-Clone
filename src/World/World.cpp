#include "World.h"

World::World() {
    for (int x = 0; x < 20; x++) {
        for (int y = 0; y < 5; y++) {
            for (int z = 0; z < 20; z++) {
                setBlock(glm::vec3(x, y, z), y == 4 ? BLOCK_TYPE::GRASS : y == 3 ? BLOCK_TYPE::DIRT : BLOCK_TYPE::STONE);
            }
        }
    }
}

World::~World()
{
    for (auto& block : blocks) {
        block->~Block();
    }
}

void World::Render(GLuint shader)
{
    for (auto& block : blocks) {
        block->Render(shader);
    }
}

Block* World::getBlock(glm::vec3 pos)
{
    for (auto& block : blocks) {
        if (block->getPos() == pos) return block;
    }

    return nullptr;
}

void World::setBlock(glm::vec3 pos, BLOCK_TYPE type)
{
    for (auto& block : blocks) {
        if (block->getPos() == pos) {
            delete block;
            break;
        }
    }
    if (type == BLOCK_TYPE::AIR) return;

    blocks.push_back(new Block(type, pos));
}
