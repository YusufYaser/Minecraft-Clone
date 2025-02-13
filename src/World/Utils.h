#pragma once

#include <glm/glm.hpp>

inline std::size_t combineHashes(std::size_t hash1, std::size_t hash2);
std::uint16_t hashPosForChunk(const glm::ivec3& pos);
std::size_t hashPos(const glm::ivec3& pos);
std::size_t hashPos(const glm::ivec2& pos);

glm::ivec2 getPosChunk(const glm::ivec3& pos);
