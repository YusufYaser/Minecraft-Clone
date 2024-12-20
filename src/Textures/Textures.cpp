#include "Textures.h"

using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;

std::map<std::string, Texture*> textures = {};

void initializeTextures() {
	stbi_set_flip_vertically_on_load(true);

	glActiveTexture(GL_TEXTURE0);

	int c = 0;
	for (const auto& dirEntry : recursive_directory_iterator("assets/textures")) {
		if (dirEntry.is_directory()) continue;

		int width, height, numCh;
		unsigned char* data = stbi_load(dirEntry.path().string().c_str(), &width, &height, &numCh, 0);
		if (!data) {
			error("Failed to load texture: ", dirEntry.path().string());
			continue;
		}

		GLuint ID;

		glGenTextures(1, &ID);
		glBindTexture(GL_TEXTURE_2D, ID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data);

		glBindTexture(GL_TEXTURE_2D, 0);

		std::string name = dirEntry.path().stem().string();

		Texture* tex = new Texture();
		tex->id = ID;
		tex->height = height;
		tex->width = width;

		textures[name] = tex;

		c++;
	}

	print("Loaded", c, "textures");
}

Texture* getTexture(std::string name) {
	return textures[name];
}
