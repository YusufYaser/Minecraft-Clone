#include "Structures.h"
#include "../Game/Game.h"

Structure* Structure::structures[STRUCTURES_COUNT];
bool Structure::initialized = false;

void Structure::initialize() {
	if (initialized) {
		error("Structures are already initialized");
		return;
	}

	print("Initializing trees");
	Structure* tree = new Structure(.0625f, { 5, 5, 5 }, [](glm::ivec2 sPos, glm::ivec3 vPos) {
		// no blocks will be placed on corners
		if ((vPos.x == 0 || vPos.x == 4) && (vPos.z == 0 || vPos.z == 4)) return BLOCK_TYPE::NONE;

		// logs
		if (vPos.y <= 2 && vPos.x == 2 && vPos.z == 2) return BLOCK_TYPE::OAK_LOG;
		
		// leaves bottom part
		if (vPos.y >= 2 && vPos.y <= 3) return BLOCK_TYPE::OAK_LEAVES;

		// leaves top part
		if (vPos.x < 1 || vPos.x > 3 || vPos.z < 1 || vPos.z > 3) return BLOCK_TYPE::NONE;
		if (vPos.y >= 2) return BLOCK_TYPE::OAK_LEAVES;

		return BLOCK_TYPE::NONE;
	});
	structures[(int)STRUCTURE_TYPE::TREE] = tree;

	initialized = true;
}

Structure::Structure(float probability, glm::ivec3 size, std::function<BLOCK_TYPE(glm::ivec2, glm::ivec3)> algorithm) {
	m_probability = probability;
	m_size = size;
	m_getBlock = algorithm;
}

BLOCK_TYPE Structure::getBlock(glm::ivec3 pos)
{
	glm::ivec2 sPos = {
		(pos.x >= 0) ? pos.x / m_size.x : (pos.x - m_size.x + 1) / m_size.x,
		(pos.z >= 0) ? pos.z / m_size.z : (pos.z - m_size.z + 1) / m_size.z
	};

	World* world = Game::getInstance()->getWorld();

	if (world->random(sPos) > m_probability) return BLOCK_TYPE::NONE;

	glm::ivec3 vPos = {
		pos.x - sPos.x * m_size.x,
		pos.y - getBase({ pos.x, pos.z }),
		pos.z - sPos.y * m_size.z
	};
	return m_getBlock(sPos, vPos);
}

int Structure::getBase(glm::ivec2 pos)
{
	glm::ivec2 sPos = {
		(pos.x >= 0) ? pos.x / m_size.x : (pos.x - m_size.x + 1) / m_size.x,
		(pos.y >= 0) ? pos.y / m_size.z : (pos.y - m_size.z + 1) / m_size.z
	};

	World* world = Game::getInstance()->getWorld();

	return world->getHeight({
		(sPos.x * m_size.x) + m_size.x / 2,
		(sPos.y * m_size.z) + m_size.z / 2
	});
}

int Structure::getHeight()
{
	return m_size.y;
}

Structure* Structure::getStructure(STRUCTURE_TYPE type)
{
	return structures[(int)type];
}
