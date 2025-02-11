#pragma once

#include <stb/stb_image.h>
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
};

void initializeTextures();
Texture* getTexture(std::string name);
