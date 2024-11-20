#include "Structures.h"
#include "../Game/Game.h"

Structure* Structure::structures[STRUCTURES_COUNT];
bool Structure::initialized = false;

void Structure::initialize() {
	if (initialized) {
		error("Structures are already initialized");
		return;
	}

	print("Initializing tree");
	StructureConfig treeConfig;
	treeConfig.priority = 0;
	treeConfig.probability = .0625f;
	treeConfig.size = { 5, 5, 5 };
	treeConfig.pivot = { 2, 0, 2 };
	Structure* tree = new Structure(STRUCTURE_TYPE::TREE, treeConfig, [](glm::ivec2 sPos, glm::ivec3 vPos) {
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

	print("Initializing lake");
	StructureConfig lakeConfig;
	lakeConfig.priority = 1;
	lakeConfig.probability = .05f;
	lakeConfig.size = { 15, 3, 15 };
	lakeConfig.pivot = { 7, 3, 7 };
	Structure* lake = new Structure(STRUCTURE_TYPE::LAKE, lakeConfig, [](glm::ivec2 sPos, glm::ivec3 vPos) {
		int depth, sizeX, sizeY;
		std::uint32_t posHash = 0;
		posHash ^= sPos.x + 0x9e3779b9 + (posHash << 6) + (posHash >> 2);
		posHash ^= sPos.y + 0x9e3779b9 + (posHash << 6) + (posHash >> 2);
		{
			std::uniform_real_distribution<> dist(2.0, 4.0);
			std::mt19937 rng(0 ^ posHash);
			depth = round(dist(rng));
		}
		{
			std::uniform_real_distribution<> dist(4.0, 12.0);
			std::mt19937 rng(0 ^ posHash);
			sizeX = round(dist(rng));
		}
		{
			std::uniform_real_distribution<> dist(4.0, 12.0);
			std::mt19937 rng(0 ^ posHash);
			sizeY = round(dist(rng));
		}

		float distFromCenterX = abs(vPos.x - sizeX / 2.0f);
		float distFromCenterY = abs(vPos.z - sizeY / 2.0f);
		float normalizedDist = sqrt((distFromCenterX * distFromCenterX) + (distFromCenterY * distFromCenterY)) / (sizeX / 2.0f);

		int adjustedDepth = round(depth * (normalizedDist));

		if (vPos.y == adjustedDepth) return BLOCK_TYPE::GRASS;
		if (vPos.y < adjustedDepth) return BLOCK_TYPE::NONE;

		return BLOCK_TYPE::WATER;
		});
	structures[(int)STRUCTURE_TYPE::LAKE] = lake;

	initialized = true;
}

Structure::Structure(STRUCTURE_TYPE type, StructureConfig& config, std::function<BLOCK_TYPE(glm::ivec2, glm::ivec3)> algorithm) {
	m_probability = config.probability;
	m_size = config.size;
	m_getBlock = algorithm;
	m_type = type;
	m_priority = config.priority;
	m_pivot = config.pivot;
}

BLOCK_TYPE Structure::getBlock(glm::ivec3 pos) {
	glm::ivec2 sPos = {
		(pos.x >= 0) ? pos.x / m_size.x : (pos.x - m_size.x + 1) / m_size.x,
		(pos.z >= 0) ? pos.z / m_size.z : (pos.z - m_size.z + 1) / m_size.z
	};

	if (!isInXZ({ pos.x, pos.z })) return BLOCK_TYPE::NONE;

	int base = getBase({ pos.x, pos.z });
	if (pos.y < base) return BLOCK_TYPE::NONE;
	if (pos.y > getHeight() + base - 1) return BLOCK_TYPE::NONE;

	glm::ivec3 vPos = {
		pos.x - sPos.x * m_size.x,
		pos.y - base,
		pos.z - sPos.y * m_size.z
	};
	return m_getBlock(sPos, vPos);
}

bool Structure::isInXZ(glm::ivec2 pos) {
	glm::ivec2 sPos = {
		(pos.x >= 0) ? pos.x / m_size.x : (pos.x - m_size.x + 1) / m_size.x,
		(pos.y >= 0) ? pos.y / m_size.z : (pos.y - m_size.z + 1) / m_size.z
	};
	World* world = Game::getInstance()->getWorld();

	return world->random(sPos, 4u * (int)m_type) <= m_probability;
}

int Structure::getBase(glm::ivec2 pos) {
	glm::ivec2 sPos = {
		(pos.x >= 0) ? pos.x / m_size.x : (pos.x - m_size.x + 1) / m_size.x,
		(pos.y >= 0) ? pos.y / m_size.z : (pos.y - m_size.z + 1) / m_size.z
	};

	World* world = Game::getInstance()->getWorld();

	return world->getHeight({
		(sPos.x * m_size.x) + m_pivot.x,
		(sPos.y * m_size.z) + m_pivot.z
	}) - m_pivot.y;
}
