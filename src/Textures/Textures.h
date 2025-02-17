#pragma once

#include <filesystem>
#include "../Logging.h"
#ifndef __gl_h_
#include <glad/gl.h>
#endif
#include <map>
#include <string>

struct Texture {
	GLuint id;
	int height;
	int width;
	glm::vec4 lod[6];
};

struct TextureAtlas {
	glm::ivec2 size = glm::ivec2();
	glm::vec4 ranges[32] = {};
	Texture* tex;
};

TextureAtlas* initializeTextures();
Texture* getTexture(std::string name);
