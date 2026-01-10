#include "World.h"
#include "../Game/Game.h"
#include <algorithm>
#include <execution>

void World::instancesPreparer(int c) {
	std::vector<Instance*> instancesCache[64];
	Player* player = nullptr;
	Game* game = Game::getInstance();

	glm::ivec3 maxDist = glm::ivec3(24, 12, 24);

	while (!unloading.load()) {
		if (!rendering.load()) continue;

		if (player == nullptr) player = Game::getInstance()->getPlayer();
		if (player == nullptr) continue;

		glm::vec3 pos = player->getCameraPos();

		renderingQueueMutex.lock();
		Chunk* chunk = nullptr;
		if (!renderingQueue.empty()) {
			chunk = renderingQueue.front();
			renderingQueue.pop_front();
		}
		renderingQueueMutex.unlock();

		if (chunk == nullptr) { // empty queue
			rendered[c].store(true);
			continue;
		}

		if (!chunk->renderingGroupsMutex.try_lock()) continue;

		for (auto& block : chunk->blocksToRender) {
			BLOCK_TYPE type = block->getType();

			const glm::ivec3& bPos = block->getPos();
			const glm::vec3 diff = glm::vec3(bPos) - pos;
			uint8_t hiddenFaces = block->hiddenFaces;
			bool transparent = isBlockTypeTransparent(type);

			BLOCK_STRUCTURE_TYPE structType = getStructureType(type);

			if (!transparent && (bPos.x % 2) == 0 && structType == BLOCK_STRUCTURE_TYPE::FULL_BLOCK) {
				for (int i = 0; i < 6; i++) {
					if ((hiddenFaces & (1 << i))) continue;
					glm::ivec3 faceDir = getBlockFaceDirection(BLOCK_FACE(i));
					glm::ivec3 d = glm::ivec3(diff) * faceDir;

					if (d.x > maxDist.x || d.y > maxDist.y || d.z > maxDist.z) {
						hiddenFaces += 1 << i;
					}
				}

				if (hiddenFaces == 63) continue;
			}

			const float half = .5f;

			if (structType == BLOCK_STRUCTURE_TYPE::PLANT) hiddenFaces = 0;

			glm::ivec3 extend = {};
			glm::ivec4 blockTypes = { type, 0, 0, 0 };

			if (game->getMergeSize() != MergeSize::None && !generatingChunks.load() && (hiddenFaces == 31 || hiddenFaces == 47)) {
				Block* otherBlock = nullptr;

				bool isWater = type == BLOCK_TYPE::WATER;

				if (bPos.x % 2 && (otherBlock = getBlock(bPos + glm::ivec3(-1, 0, 0)))) {
					if ((isWater == (otherBlock->getType() == BLOCK_TYPE::WATER)) && otherBlock->hiddenFaces == hiddenFaces) continue;
				} else if ((bPos.x % 2) == 0 && (otherBlock = getBlock(bPos + glm::ivec3(1, 0, 0)))) {
					if ((isWater == (otherBlock->getType() == BLOCK_TYPE::WATER)) && otherBlock->hiddenFaces == hiddenFaces) {
						extend = { 1, 0, 0 };
						blockTypes[1] = (int)otherBlock->getType();
						if (game->getMergeSize() == MergeSize::TwoByTwo) {
							if ((bPos.z % 2) == 0) {
								if ((otherBlock = getBlock(bPos + glm::ivec3(0, 0, 1)))) {
									if ((isWater == (otherBlock->getType() == BLOCK_TYPE::WATER)) && otherBlock->hiddenFaces == hiddenFaces) {
										BLOCK_TYPE prevBlockType = otherBlock->getType();
										if ((otherBlock = getBlock(bPos + glm::ivec3(1, 0, 1)))) {
											if ((isWater == (otherBlock->getType() == BLOCK_TYPE::WATER)) && otherBlock->hiddenFaces == hiddenFaces) {
												extend = { 1, 0, 1 };
												blockTypes[2] = (int)prevBlockType;
												blockTypes[3] = (int)otherBlock->getType();;
											}
										}
									}
								}
							} else {
								if ((otherBlock = getBlock(bPos + glm::ivec3(0, 0, -1)))) {
									if ((isWater == (otherBlock->getType() == BLOCK_TYPE::WATER)) && otherBlock->hiddenFaces == hiddenFaces) {
										if ((otherBlock = getBlock(bPos + glm::ivec3(1, 0, -1)))) {
											if ((isWater == (otherBlock->getType() == BLOCK_TYPE::WATER)) && otherBlock->hiddenFaces == hiddenFaces) {
												continue;
											}
										}
									}
								}
							}
						}
					}
				}
			}

			Instance* i = nullptr;
			for (auto& inst : instancesCache[hiddenFaces]) {
				if (inst->offsets.size() >= MAX_INSTANCE_OFFSETS) continue;
				if (inst->transparent != transparent) continue;
				if (inst->bStructType != structType) continue;
				i = inst;
				break;
			}
			if (i == nullptr) {
				i = new Instance();
				i->hiddenFaces = hiddenFaces;
				i->transparent = transparent;
				i->bStructType = structType;

				instancesMutex.lock();
				instances.push_back(i);
				instancesMutex.unlock();

				instancesToInitMutex.lock();
				instancesToInit.push_back(i);
				instancesToInitMutex.unlock();

				instancesCache[hiddenFaces].push_back(i);
			}

			i->offsets.push_back(BlockOffsetData{ bPos, blockTypes, extend });
		}

		chunk->renderingGroupsMutex.unlock();
	}
}

void World::render() {
	m_chunksRendered = 0;
	m_instancesRendered = 0;

	Player* player = Game::getInstance()->getPlayer();
	if (player == nullptr) return;

	glm::mat4 playerView = player->getView();
	glm::mat4 playerProjection = player->getProjection();
	glm::vec3 pos = player->getCameraPos();

	static Block* skybox;
	static Block* clouds;
	static Block* sun;
	static Block* moon;
	if (skybox == nullptr) {
		skybox = new Block(BLOCK_TYPE::STONE, glm::ivec3(0, 0, 0), 0);
	}
	if (clouds == nullptr) {
		clouds = new Block(BLOCK_TYPE::STONE, glm::ivec3(0, 0, 0), 0);
	}
	if (sun == nullptr) {
		sun = new Block(BLOCK_TYPE::STONE, glm::ivec3(0, 0, 0), 0);
	}
	if (moon == nullptr) {
		moon = new Block(BLOCK_TYPE::STONE, glm::ivec3(0, 0, 0), 0);
	}

	Shader* shader = Game::getInstance()->getShader();
	Shader* skyboxShader = Game::getInstance()->getSkyboxShader();
	skyboxShader->activate();
	skyboxShader->setUniform("view", glm::lookAt(glm::vec3(), !player->inFreecam() ? player->getOrientation() : player->getFreecamOrientation(), player->getUp()));
	skyboxShader->setUniform("projection", playerProjection);
	skyboxShader->setUniform("ambientLight", getAmbientLight());

	glBindTexture(GL_TEXTURE_2D, getTexture("skybox_day")->id);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, getTexture("skybox_night")->id);
	glActiveTexture(GL_TEXTURE0);
	skyboxShader->setUniform("blockPos", glm::vec3(0, 0, 0));
	skyboxShader->setUniform("type", 0);
	skybox->Render(nullptr, 0, false);

	glBindTexture(GL_TEXTURE_2D, getTexture("sun")->id);
	skyboxShader->setUniform("blockPos", glm::vec3(0, .9f, 0));
	skyboxShader->setUniform("type", 2);
	glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians((m_tick / 24000.0f) * 360.0f), glm::vec3(0, 0, 1));
	skyboxShader->setUniform("model", model);
	sun->Render(nullptr, 47, false);

	glBindTexture(GL_TEXTURE_2D, getTexture("moon")->id);
	model = glm::rotate(glm::mat4(1.0f), glm::radians((m_tick / 24000.0f - .5f) * 360.0f), glm::vec3(0, 0, 1));
	skyboxShader->setUniform("model", model);
	moon->Render(nullptr, 47, false);

	glBindTexture(GL_TEXTURE_2D, getTexture("clouds")->id);
	skyboxShader->setUniform("blockPos", glm::vec3(0, .75f - (pos.y / (MAX_HEIGHT + 32) / 4), 0));
	skyboxShader->setUniform("playerPos", player->getPos());
	skyboxShader->setUniform("type", 1);
	clouds->Render(nullptr, 47, false);

	shader->activate();
	shader->setUniform("view", playerView);
	shader->setUniform("projection", playerProjection);

	if (Game::getInstance()->isGuiEnabled()) {
		player->getTargetBlock(&targetBlock);
	} else {
		targetBlock = nullptr;
	}
	if (targetBlock != nullptr) {
		shader->setUniform("highlighted", targetBlock->getPos());
	} else {
		shader->setUniform("highlighted", glm::ivec3(0, -1, 0));
	}

	int renderDistance = Game::getInstance()->getRenderDistance();
	shader->setUniform("fogSize", 2.0f);
	shader->setUniform("playerPos", pos);

	glm::ivec2 playerChunk = getPosChunk(player->getPos());

	shader->setUniform("ambientLight", getAmbientLight());

	static std::vector<Chunk*> chunksToRender;

	static glm::ivec2 oldPlayerChunk = playerChunk;
	static size_t oldChunksLoaded = 0;
	static bool wasGenerating = false;
	size_t chunksLoaded = this->chunksLoaded() - this->chunkLoadQueueCount();
	bool rerender = false;
	static int lastYLevel = 0;

	static int oldChunksRendered = 0;
	bool isGenerating = generatingChunks.load();

	if (oldPlayerChunk != playerChunk || lastYLevel != round(pos.y / 8) || oldChunksLoaded < chunksLoaded || isGenerating != wasGenerating || m_worldRenderModified) {
		chunksToRender.clear();

		for (int x = -renderDistance + playerChunk.x - PRELOAD_DISTANCE; x < renderDistance + playerChunk.x + PRELOAD_DISTANCE; x++) {
			for (int y = -renderDistance + playerChunk.y - PRELOAD_DISTANCE; y < renderDistance + playerChunk.y + PRELOAD_DISTANCE; y++) {
				glm::ivec2 cPos = glm::ivec2(x, y);

				if (glm::length(glm::vec2(cPos - playerChunk)) > renderDistance + PRELOAD_DISTANCE) {
					chunksMutex.unlock();
					continue;
				}

				std::size_t chunkCh = hashPos(cPos);
				chunksMutex.lock();
				std::unordered_map<std::size_t, Chunk*>::iterator it = chunks.find(chunkCh);
				if (it == chunks.end()) {
					chunksMutex.unlock();
					loadChunk(cPos);
					continue;
				}

				if (glm::length(glm::vec2(cPos - playerChunk) * 16.0f) + .5f > renderDistance * 16.0f) {
					chunksMutex.unlock();
					continue;
				}

				Chunk* chunk = it->second;
				chunksMutex.unlock();
				if (chunk == nullptr) continue;
				if (!chunk->loaded) continue;

				chunksToRender.push_back(chunk);
			}
		}

		rerender = oldChunksRendered != chunksToRender.size() || oldPlayerChunk != playerChunk || isGenerating != wasGenerating || lastYLevel != round(pos.y / 8) || m_worldRenderModified;

		oldPlayerChunk = playerChunk;
		oldChunksLoaded = chunksLoaded;
		lastYLevel = round(pos.y / 8);
		wasGenerating = isGenerating;

		if (!rerender) {
			chunksToRender.clear();
		}
	}
	int c = 0;

	if (rerender || m_worldRenderModified) {
		for (auto& chunk : chunksToRender) {
			glm::ivec2 cPos = chunk->pos;

			m_chunksRendered++;

			renderingQueue.push_back(chunk);
			rendered[c] = false;

			c = (c + 1) % INSTANCES_PREP_THREAD_COUNT;

			chunk->lastRendered = time(nullptr);
		}

		for (auto& i : instances) {
			i->offsets.resize(0);
		}

		rendering.store(true);
		for (int c = 0; c < INSTANCES_PREP_THREAD_COUNT; c++) {
			while (!rendered[c].load());
		}
		rendering.store(false);

		std::partition(instances.begin(), instances.end(), [](const Instance* i) {
			return !i->transparent;
			});

		oldChunksRendered = m_chunksRendered;
	} else {
		m_chunksRendered = oldChunksRendered;
	}

	glEnable(GL_DEPTH_TEST);
	for (auto& entity : entities) {
		if (glm::length(pos - entity->getPos()) > (renderDistance - 1) * 16) continue;

		if (entity == player) {
			if (player->inFreecam()) player->render();
		} else {
			entity->render();
		}
	}
	shader->activate();

	while (instancesToInit.size() != 0) {
		Instance* i = instancesToInit.front();
		instancesToInit.erase(instancesToInit.begin());

		BlockStructureData* data = createBlockStructureData(i->bStructType, i->hiddenFaces);
		i->bStructData = data;

		glBindVertexArray(data->VAO);

		glGenBuffers(1, &i->VBO);
		glBindBuffer(GL_ARRAY_BUFFER, i->VBO);
		glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(BlockOffsetData), 0);
		glVertexAttribIPointer(4, 4, GL_INT, sizeof(BlockOffsetData), (void*)offsetof(BlockOffsetData, blockTypes));
		glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(BlockOffsetData), (void*)offsetof(BlockOffsetData, extend));
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);

		//debug("Initialized new instance:", i, "Hidden faces:", int(i->hiddenFaces));
	}

	TextureAtlas* atlas = Game::getInstance()->getTexAtlas();
	glBindTexture(GL_TEXTURE_2D, atlas->tex->id);

	static bool blend = true;

	m_blocksRendered = 0;
	for (auto& i : instances) {
		if (i->offsets.size() == 0 || i->bStructData == nullptr) continue;
		if (i->transparent && !blend) {
			glEnable(GL_BLEND);
			glDisable(GL_CULL_FACE);
			blend = true;
		} else if (!i->transparent && blend) {
			glDisable(GL_BLEND);
			glEnable(GL_CULL_FACE);
			blend = false;
		}

		glBindBuffer(GL_ARRAY_BUFFER, i->VBO);
		glBindVertexArray(i->bStructData->VAO);

		if (rerender || m_worldRenderModified) glBufferData(GL_ARRAY_BUFFER, i->offsets.size() * sizeof(BlockOffsetData), i->offsets.data(), GL_DYNAMIC_DRAW);

		glDrawElementsInstanced(GL_TRIANGLES, i->bStructData->faceCount * 6, GL_UNSIGNED_BYTE, 0, i->offsets.size());

		m_instancesRendered++;
		m_blocksRendered += i->offsets.size();
	}
	glDisable(GL_DEPTH_TEST);

	if (!blend) {
		glEnable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		blend = true;
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	if (m_worldRenderModified) m_worldRenderModified = false;
}