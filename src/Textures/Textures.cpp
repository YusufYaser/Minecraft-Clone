#include <vector>
#include <algorithm>
#include "../Block/Block.h"
#include "../Logging.h"
#include "Textures.h"
#include <stb/stb_image.h>

#define ATLAS_WIDTH 1024
#define ATLAS_HEIGHT 1024

using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;

std::map<std::string, Texture*> textures = {};

TextureAtlas* initializeTextures() {
	for (auto& [name, tex] : textures) {
		glDeleteTextures(1, &tex->id);
		tex->id = 0;
	}

	stbi_set_flip_vertically_on_load(true);

	glActiveTexture(GL_TEXTURE0);
	GLuint ID;

	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D, ID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	uint8_t invalidTexData[2 * 2 * 4]{};
	for (int x = 0; x < 2; x++) {
		for (int y = 0; y < 2; y++) {
			int pn = ((x * 2) + y) * 4;
			if ((x == 1) != (y == 1)) {
				invalidTexData[pn] = 255;
			}
			invalidTexData[pn + 3] = 255;
		}
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, &invalidTexData);
	glGenerateMipmap(GL_TEXTURE_2D);

	Texture* tex = textures["invalid"];
	if (tex == nullptr) tex = new Texture();
	tex->id = ID;
	tex->height = 2;
	tex->width = 2;

	textures["invalid"] = tex;

	Texture* black = new Texture();
	textures["black"] = black;

	recursive_directory_iterator it;
	try {
		it = recursive_directory_iterator("assets/textures");
	} catch (std::filesystem::filesystem_error e) {
		error("Failed to load textures:", e.what());
		return nullptr;
	}

	TextureAtlas* atlas = new TextureAtlas();
	Texture* atlasTex = textures["atlas"];
	if (atlasTex == nullptr) atlasTex = new Texture();

	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D, ID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ATLAS_WIDTH, ATLAS_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	atlasTex->id = ID;
	atlasTex->height = ATLAS_HEIGHT;
	atlasTex->width = ATLAS_WIDTH;

	atlas->size = { ATLAS_WIDTH, ATLAS_HEIGHT };
	atlas->tex = atlasTex;

	for (int i = 0; i < 32; i++) {
		atlas->ranges[i] = glm::vec4();
	}

	int atlasWidth = 0;
	int atlasHeight = 0;
	int atlasMaxHeight = 0;

	int c = 0;
	for (const auto& dirEntry : it) {
		if (dirEntry.is_directory()) continue;

		int width, height, numCh;
		unsigned char* data = stbi_load(dirEntry.path().string().c_str(), &width, &height, &numCh, 0);
		if (!data) {
			error("Failed to load texture:", dirEntry.path().string());
			continue;
		}

		std::string name = dirEntry.path().stem().string();

		for (int i = 0; i < BLOCK_TYPE_COUNT; i++) {
			if (getTextureName((BLOCK_TYPE)i) != name) continue;

			if (width > ATLAS_WIDTH) {
				atlasWidth = 0;
				atlasHeight += atlasMaxHeight;
				atlasMaxHeight = 0;
			}

			glBindTexture(GL_TEXTURE_2D, atlasTex->id);
			glTexSubImage2D(GL_TEXTURE_2D, 0, atlasWidth, atlasHeight, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

			atlas->ranges[i] = glm::vec4(
				atlasWidth / (float)ATLAS_WIDTH, atlasHeight / (float)ATLAS_HEIGHT,
				(atlasWidth + width) / (float)ATLAS_WIDTH, (atlasHeight + height) / (float)ATLAS_HEIGHT
			);

			atlasWidth += width;
			atlasMaxHeight = std::max(height, atlasMaxHeight);

			debug("Added texture to atlas:", name);
			break;
		}

		if (name.ends_with(".lut")) {
			if (width != 512 || height != 512) {
				error("Texture size must be 512x512 in", name);

				stbi_image_free(data);
				continue;
			}

			glGenTextures(1, &ID);
			glBindTexture(GL_TEXTURE_3D, ID);

			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			unsigned char* lutData = new unsigned char[width * height * numCh];

			int gridSize = width / 8;

			for (int b = 0; b < gridSize; b++) {
				int gridX = b % 8;
				int gridY = b / 8;

				for (int r = 0; r < gridSize; r++) {
					for (int g = 0; g < gridSize; g++) {
						int srcIndex = ((gridY * width * gridSize) + (g * width) + ((8 - gridX - 1) * gridSize) + (r)) * numCh;
						int dstIndex = (((gridSize - b - 1) * gridSize * gridSize) + ((gridSize - g - 1) * gridSize) + r) * 3;

						lutData[dstIndex] = data[srcIndex];
						lutData[dstIndex + 1] = data[srcIndex + 1];
						lutData[dstIndex + 2] = data[srcIndex + 2];
					}
				}
			}

			glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB8, gridSize, gridSize, gridSize, 0, GL_RGB, GL_UNSIGNED_BYTE, lutData);
			glGenerateMipmap(GL_TEXTURE_3D);

			delete[] lutData;
		} else {
			glGenTextures(1, &ID);
			glBindTexture(GL_TEXTURE_2D, ID);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, numCh == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		Texture* tex = textures[name];
		if (tex == nullptr) tex = new Texture();
		tex->id = ID;
		tex->height = height;
		tex->width = width;

		for (auto& l : tex->lod) {
			l = glm::vec4();
		}

		if (height == 96) {
			for (int x = 0; x < width; x++) {
				for (int y = 0; y < height; y++) {
					unsigned char* pixel = data + y * width + x * 4;
					int i = (int)floor(y / 16.0f);
					i += 2;
					i %= 6;
					tex->lod[i] += glm::vec4(float(*pixel), float(*(pixel + 1)), float(*(pixel + 2)), float(*(pixel + 3))) / float(width * height / 6.0f) / 255.0f;
				}
			}
		}

		stbi_image_free(data);

		textures[name] = tex;

		c++;
	}
	glBindTexture(GL_TEXTURE_2D, atlasTex->id);
	glGenerateMipmap(GL_TEXTURE_2D);
	textures["atlas"] = atlasTex;
	glBindTexture(GL_TEXTURE_2D, 0);

	print("Loaded", c, "textures");

	return atlas;
}

Texture* getTexture(std::string name) {
	auto it = textures.find(name);
	if (it == textures.end()) {
		static std::vector<std::string> names;
		if (std::find(names.begin(), names.end(), name) == names.end()) {
			warn("Couldn't find texture:", name);
			names.push_back(name);
		}
		return textures["invalid"];
	}
	return it->second;
}
