#pragma once

#include <stb/stb_image.h>
#include <filesystem>
#include "../Logging.h"
#include <glad/gl.h>
#include <map>
#include <string>

void initializeTextures();
GLuint getTexture(std::string name);
