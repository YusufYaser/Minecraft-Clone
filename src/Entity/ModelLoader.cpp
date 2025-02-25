#include "Entity.h"
#include <fstream>
#include <algorithm>
#include <json/json.hpp>
#include "../Logging.h"

inline EntityModel* models[ENTITY_TYPES_COUNT];

struct Vertex {
	glm::vec3 pos;
	glm::vec2 texCoord;
	glm::vec3 normal;
};

EntityModel* getEntityModel(ENTITY_TYPE& type) {
	return models[(int)type];
}

const char* getModelName(ENTITY_TYPE& type) {
	switch (type) {
	case ENTITY_TYPE::PLAYER:
		return "player";
	default:
		return "invalid";
	}
}

EntityModel* createModel(std::vector<Vertex>& vertices, std::vector<GLuint>& indices) {
	GLuint VAO, VBO, EBO;

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)offsetof(Vertex, texCoord));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	EntityModel* model = new EntityModel();
	model->VAO = VAO;
	model->indicesCount = indices.size();

	return model;
}

inline bool modelsLoaded = false;
void loadEntityModels() {
	if (modelsLoaded) {
		error("Entity models are already loaded");
		return;
	}

	for (int i = 0; i < ENTITY_TYPES_COUNT; i++) {
		ENTITY_TYPE type = ENTITY_TYPE(i);
		debug("Loading", getModelName(type), "model");
		try {
			std::string path = "assets/models/" + std::string(getModelName(type)) + ".json";
			std::ifstream file(path);
			if (!file.is_open()) {
				error("Failed to open", getModelName(type), "model file");
				continue;
			}

			nlohmann::json json = nlohmann::json::parse(file, nullptr, false, true);
			file.close();

			if (!json.contains("p") || !json.contains("f") ||
				!json["p"].is_array() || !json["f"].is_array()) {
				error("Model file is invalid:", path);
				continue;
			}

			bool error = false;

			std::vector<glm::vec3> positions;

			debug("Loading positions");
			int c = -1;
			for (const auto& p : json["p"]) {
				c++;
				if (!p.is_array() || p.size() != 3 ||
					!p[0].is_number() || !p[1].is_number() || !p[2].is_number()
					) {
					debug("Position", c, "is invalid");
					error = true;
					break;
				}

				positions.push_back(glm::vec3(p[0], p[1], p[2]));
			}

			if (error) {
				error("Failed to load", path);
				continue;
			}

			debug("Loaded", positions.size(), "positions");

			debug("Loading faces");

			std::vector<Vertex> vertices;
			std::vector<unsigned int> indices;

			c = -1;
			for (const auto& f : json["f"]) {
				c++;
				if (!f.is_object() ||
					!f.contains("v") || !f["v"].is_array() ||
					!f.contains("n") || !f["n"].is_array()) {
					debug("Face", c, "is invalid");
					error = true;
					break;
				}

				auto& n = f["n"];
				if (n.size() != 3 || !n[0].is_number() || !n[1].is_number() || !n[2].is_number()) {
					debug("Face", c, "normal is invalid");
					error = true;
					break;
				}

				glm::vec3 normal = glm::normalize(glm::vec3(n[0], n[1], n[2]));

				int vi = -1;
				for (const auto& v : f["v"]) {
					vi++;
					if (!v.is_object() ||
						!v.contains("p") || !v["p"].is_number() ||
						!v.contains("t") || !v["t"].is_array()) {
						debug("Face", i, "vertex", vi, "is invalid");
						error = true;
						break;
					}

					if (v["p"] >= positions.size()) {
						debug("Face", i, "vertex", vi, "position is invalid");
						error = true;
						break;
					}

					auto& t = v["t"];
					if (t.size() != 2 || !t[0].is_number() || !t[1].is_number()) {
						debug("Face", i, "vertex", vi, "texcoord is invalid");
						error = true;
						break;
					}

					Vertex vertex{};
					vertex.pos = positions[v["p"]];
					vertex.texCoord = glm::vec2(t[0], t[1]);
					vertex.normal = normal;

					vertices.push_back(vertex);
					indices.push_back(static_cast<unsigned int>(indices.size()));
				}
				if (error) break;
			}

			debug("Loaded", indices.size() / 3, "triangles");

			if (error) {
				error("Failed to load", path);
				continue;
			}

			debug("Creating model");

			models[i] = createModel(vertices, indices);
		} catch (nlohmann::json::exception e) {
			error("Failed to parse", getModelName(type), "model:", e.what());
			continue;
		} catch (std::exception e) {
			error("Failed to load", getModelName(type), "model:", e.what());
			continue;
		}

		debug("Loaded", getModelName(type), "model");
	}
}