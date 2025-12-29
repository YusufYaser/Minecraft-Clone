#include "World.h"
#include "Utils.h"

inline std::size_t combineHashes(std::size_t hash1, std::size_t hash2) {
	return hash1 ^ (hash2 + 0x9e3779b9 + (hash1 << 6) + (hash1 >> 2));
}

std::uint16_t hashPosForChunk(const glm::ivec3& pos) {
	glm::ivec2 cPos = getPosChunk(pos);
	glm::ivec3 pos2 = {
		pos.x - (cPos.x * 16),
		pos.y,
		pos.z - (cPos.y * 16),
	};
	return (pos2.y * 16 * 16) + (pos2.z * 16) + pos2.x;
}

static inline std::size_t hash_int32(int32_t value) {
	const uint8_t* data = reinterpret_cast<const uint8_t*>(&value);
	std::size_t hash = 14695981039346656037ULL;
	for (std::size_t i = 0; i < sizeof(value); ++i) {
		hash ^= data[i];
		hash *= 1099511628211ULL;
	}
	return hash;
}

std::size_t hashPos(const glm::ivec3& pos) {
	std::size_t hx = hash_int32(pos.x);
	std::size_t hy = hash_int32(pos.y);
	std::size_t hz = hash_int32(pos.z);

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

int World::getHeight(glm::ivec2 pos) {
	if (generator == Generator::Flat) {
		return 7;
	}
	if (generator == Generator::Debug) {
		return 1;
	}
	if (generator == Generator::Void) {
		return 1;
	}

	const double random = perlin.octave2D_01((pos.x * 0.025), (pos.y * 0.025), 4);
	int height = static_cast<int>(round(random * 16 + getSeaLevel() - 6));

	Biome biome = getBiome(pos);
	switch (biome.type) {
	case BiomeType::Mountain:
		height += height * biome.value;
		break;
	case BiomeType::Desert:
		height = static_cast<int>(round((random * 5 + getSeaLevel()) * fmin(biome.value / 0.05f, 1.0f) + (float)height * (1.0f - fmin(biome.value / 0.05f, 1.0f))));
		break;
	}

	return height;
}

Biome World::getBiome(glm::ivec2 pos) {
	float biome = getBiomeFloat(pos);

	/*
	1.00
	|
	| Mountain
	|
	0.75
	|
	| Plains
	|
	0.25
	|
	| Desert
	|
	0.00
	*/

	if (biome >= 0.75f) {
		return Biome{ BiomeType::Mountain, (biome - 0.75f) / 0.25f };
	}
	if (biome <= 0.25f) {
		return Biome{ BiomeType::Desert, 1.0f - biome / 0.25f };
	}

	return Biome{ BiomeType::Plains, 1 };
}

float World::getBiomeFloat(glm::ivec2 pos) {
	return perlinBiomes.octave2D_01((pos.x * 0.0005), (pos.y * 0.0005), 4);
}

double World::random(glm::ivec2 pos, int otherSeed) {
	std::uniform_real_distribution<> dist(0.0, 1.0);
	std::uint32_t hash = seed;
	hash ^= pos.x + 0x9e3779b9 + (hash << 6) + (hash >> 2);
	hash ^= pos.y + 0x9e3779b9 + (hash << 6) + (hash >> 2);
	hash ^= otherSeed + 0x9e3779b9 + (hash << 6) + (hash >> 2);
	std::mt19937 rng(hash);
	return dist(rng);
}
