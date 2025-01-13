#include "Textures.h"

using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;

std::map<std::string, Texture*> textures = {};

void initializeTextures() {
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

	Texture* tex = new Texture();
	tex->id = ID;
	tex->height = 2;
	tex->width = 2;

	textures["invalid"] = tex;

	recursive_directory_iterator it;
	try {
		it = recursive_directory_iterator("assets/textures");
	} catch (std::filesystem::filesystem_error e) {
		error("Failed to load textures:", e.what());
		return;
	}

	int c = 0;
	for (const auto& dirEntry : it) {
		if (dirEntry.is_directory()) continue;

		int width, height, numCh;
		unsigned char* data = stbi_load(dirEntry.path().string().c_str(), &width, &height, &numCh, 0);
		if (!data) {
			error("Failed to load texture:", dirEntry.path().string());
			continue;
		}

		glGenTextures(1, &ID);
		glBindTexture(GL_TEXTURE_2D, ID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data);

		std::string name = dirEntry.path().stem().string();

		Texture* tex = new Texture();
		tex->id = ID;
		tex->height = height;
		tex->width = width;

		textures[name] = tex;

		c++;
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	print("Loaded", c, "textures");
}

Texture* getTexture(std::string name) {
	auto it = textures.find(name);
	if (it == textures.end()) {
		static std::vector<std::string> names;
		auto end = names.end();
		if (std::find(names.begin(), end, name) == end) {
			warn("Couldn't find texture:", name);
			names.push_back(name);
		}
		return textures["invalid"];
	}
	return it->second;
}
