#include "World.h"
#include "Utils.h"
#include "../Game/Game.h"

World::World(WorldSettings& settings) {
	World::seed = settings.seed;
	World::perlin = siv::PerlinNoise{ seed };
	World::generator = settings.generator;
	m_tick = settings.initialTick;

	int structureCount = 0;
	for (STRUCTURE_TYPE structureType : settings.structures) {
		Structure* structure = Structure::getStructure(structureType);
		structures.push_back(structure);
	}

	std::sort(structures.begin(), structures.end(), [](Structure* a, Structure* b) {
		return a->getPriority() > b->getPriority();
		});

	unloading.store(false);

	chunkLoader = std::thread([this]() {
		chunkLoaderFunc();
		});

	chunkUnloader = std::thread([this]() {
		chunkUnloaderFunc();
		});

	tickThread = std::thread([this]() {
		Game* game = Game::getInstance();

		double lastTick = glfwGetTime();
		double delta = 0;

		while (!unloading) {
			double currentTime = glfwGetTime();

			if (game->gamePaused()) {
				// maintain the tick delta until the game gets unpaused
				lastTick = currentTime - delta;
				continue;
			}

			delta = currentTime - lastTick;

			if (delta >= 1.0 / 20) {
				tick();
				lastTick = currentTime;
			}
		}
		});
}

World::~World() {
	unloading.store(true);
	print("Waiting for world tick thread...");
	tickThread.join();
	print("Waiting for chunk loader thread...");
	chunkLoader.join();
	print("Waiting for chunk unloader thread...");
	chunkUnloader.join();
}

WorldSaveData* World::createWorldSaveData() {
	WorldSaveData* data = new WorldSaveData();
	data->tick = m_tick;
	data->seed = seed;
	data->generator = generator;
	int i = 0;
	for (auto& structure : structures) {
		data->structures[i++] = structure->getType();
	}
	data->structuresCount = i;

	glm::vec3 pos = Game::getInstance()->getPlayer()->pos;
	data->playerPos[0] = pos.x;
	data->playerPos[1] = pos.y;
	data->playerPos[2] = pos.z;

	return data;
}

void World::render() {
	m_chunksRendered = 0;
	m_instancesRendered = 0;

	Player* player = Game::getInstance()->getPlayer();
	if (player == nullptr) return;

	glm::mat4 playerView = player->getView();
	glm::mat4 playerProjection = player->getProjection();

	static Block* skybox;
	if (skybox == nullptr) {
		skybox = new Block(BLOCK_TYPE::STONE, glm::ivec3(0, 0, 0), 0);
	}

	Shader* skyboxShader = Game::getInstance()->getSkyboxShader();
	skyboxShader->activate();
	skyboxShader->setUniform("view", glm::lookAt(glm::vec3(), player->orientation, player->up));
	skyboxShader->setUniform("projection", playerProjection);
	skyboxShader->setUniform("ambientLight", getAmbientLight());

	glDepthRange(0.9, 1.0);
	glBindTexture(GL_TEXTURE_2D, getTexture("skybox")->id);
	skybox->Render(skyboxShader, 0, false);
	glDepthRange(0.01, 0.9);

	Shader* shader = Game::getInstance()->getShader();

	shader->activate();
	shader->setUniform("view", playerView);
	shader->setUniform("projection", playerProjection);

	Block* targetBlock = nullptr;
	player->getTargetBlock(&targetBlock);

	if (targetBlock != nullptr) targetBlock->highlighted = true;

	// Camera Position
	glm::vec3 pos = player->getCameraPos();
	int renderDistance = Game::getInstance()->getRenderDistance();
	shader->setUniform("highlighted", false);
	shader->setUniform("isLiquidTop", false);

	float cosHalfFOV = std::cos(glm::radians(190.0f / 2.0f));

	glm::ivec2 playerChunk = getPosChunk(player->pos);

	std::unordered_map<Chunk*, std::vector<BLOCK_TYPE>> queued;

	shader->setUniform("ambientLight", getAmbientLight());

	static std::vector<Instance*> instances = {};

	for (auto& i : instances) {
		i->offsetsCount = 0;
		i->highlightedOffset = -1;
	}

	for (int x = -(renderDistance / 2) + playerChunk.x; x < (renderDistance / 2) + playerChunk.x; x++) {
		for (int y = -(renderDistance / 2) + playerChunk.y; y < (renderDistance / 2) + playerChunk.y; y++) {
			glm::ivec2 cPos = glm::ivec2(x, y);

			if (glm::length(glm::vec2(cPos - playerChunk)) > 1) {
				glm::vec3 chunkCenter = glm::vec3(x * 16 + 8, 0.0f, y * 16 + 8);
				glm::vec3 playerPos3D = pos;

				glm::vec3 toChunk = glm::normalize(chunkCenter - playerPos3D);
				glm::vec3 viewDir = glm::normalize(player->orientation);

				if (glm::dot(toChunk, viewDir) < cosHalfFOV) continue;
			}

			std::size_t chunkCh = hashPos(cPos);
			chunksMutex.lock();
			std::unordered_map<std::size_t, Chunk*>::iterator it = chunks.find(chunkCh);
			if (it == chunks.end()) {
				chunksMutex.unlock();
				loadChunk(cPos);
				continue;
			}
			Chunk* chunk = it->second;
			chunksMutex.unlock();
			if (chunk == nullptr) continue;
			if (!chunk->loaded) continue;

			if (!chunk->renderingGroupsMutex.try_lock()) continue;

			m_chunksRendered++;

			for (auto& [type, blocks] : chunk->renderingGroups) {
				if (type == BLOCK_TYPE::NONE || type == BLOCK_TYPE::AIR) continue;

				Texture* tex = getTexture(getTextureName(type));

				for (auto& block : blocks) {
					if (block == nullptr) continue;

					const glm::ivec3& bPos = block->getPos();
					const glm::vec3 diff = glm::vec3(bPos) - pos;
					const float half = .5f;

					uint8_t hiddenFaces = block->hiddenFaces;

					hiddenFaces |= (diff.x > -half) << (int)BLOCK_FACE::RIGHT;
					hiddenFaces |= (diff.x < half) << (int)BLOCK_FACE::LEFT;

					hiddenFaces |= (diff.y > -half) << (int)BLOCK_FACE::TOP;
					hiddenFaces |= (diff.y < half) << (int)BLOCK_FACE::BOTTOM;

					hiddenFaces |= (diff.z > -half) << (int)BLOCK_FACE::FRONT;
					hiddenFaces |= (diff.z < half) << (int)BLOCK_FACE::BACK;

					if (hiddenFaces >= 63) continue;

					Instance* i = nullptr;
					for (auto& inst : instances) {
						if (inst->offsetsCount >= MAX_INSTANCE_OFFSETS) continue;
						if (inst->tex->id != tex->id) continue;
						if (inst->bStructData->hiddenFaces != hiddenFaces) continue;
						i = inst;
						break;
					}
					if (i == nullptr) {
						BlockStructureData* data = createBlockStructureData(hiddenFaces);
						i = new Instance();
						instances.push_back(i);
						i->tex = tex;
						i->offsetsCount = 0;
						i->bStructData = data;
						i->blockType = type;

						glBindVertexArray(data->VAO);

						glGenBuffers(1, &i->VBO);
						glBindBuffer(GL_ARRAY_BUFFER, i->VBO);
						glBufferData(GL_ARRAY_BUFFER, MAX_INSTANCE_OFFSETS * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

						glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
						glEnableVertexAttribArray(3);
						glVertexAttribDivisor(3, 1);
					}

					if (block->highlighted) i->highlightedOffset = i->offsetsCount;

					i->offsets[i->offsetsCount++] = glm::vec3(bPos);
				}
			}
			chunk->lastRendered = time(nullptr);
			chunk->renderingGroupsMutex.unlock();
		}
	}

	std::partition(instances.begin(), instances.end(), [](const Instance* i) {
		return !isBlockTypeTransparent(i->blockType);
		});

	for (auto& i : instances) {
		if (i->offsetsCount == 0) continue;
		m_instancesRendered++;

		glBindBuffer(GL_ARRAY_BUFFER, i->VBO);
		glBindVertexArray(i->bStructData->VAO);
		glBindTexture(GL_TEXTURE_2D, i->tex->id);

		shader->setUniform("animationFrameCount", getAnimationFrameCount(i->blockType));
		shader->setUniform("isLiquidTop", i->blockType == BLOCK_TYPE::WATER);
		shader->setUniform("highlighted", i->highlightedOffset);

		glBufferSubData(GL_ARRAY_BUFFER, 0, i->offsetsCount * sizeof(glm::vec3), i->offsets);

		glDrawArraysInstanced(GL_TRIANGLES, 0, i->bStructData->faceCount * 6, i->offsetsCount);
	}

	if (targetBlock != nullptr) targetBlock->highlighted = false;
}

void World::tick() {
	m_tick++;
}

float World::getAmbientLight() {
	const double halfPi = acos(0.0); // 3.14 / 2

	float light = static_cast<float>((sin(((getTime() / 12000.0f) + 1) * halfPi) + 1.0f) / 2.0f);
	light *= 1 - .1f;
	return light + .1f;
}

int World::getTime() {
	return getTick();
}
