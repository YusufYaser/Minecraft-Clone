#include "World.h"
#include "../Game/Game.h"
#include <algorithm>
#include <execution>

void World::renderer(int c) {
	std::vector<Instance*> instancesCache[64];
	Player* player = nullptr;

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

			if (type == BLOCK_TYPE::NONE || type == BLOCK_TYPE::AIR) continue;
			bool transparent = isBlockTypeTransparent(type);

			const glm::ivec3& bPos = block->getPos();
			const glm::vec3 diff = glm::vec3(bPos) - pos;
			const float half = .5f;

			uint8_t hiddenFaces = block->hiddenFaces;

			/*hiddenFaces |= (diff.x > -half) << (int)BLOCK_FACE::RIGHT;
			hiddenFaces |= (diff.x < half) << (int)BLOCK_FACE::LEFT;

			hiddenFaces |= (diff.y > -half) << (int)BLOCK_FACE::TOP;
			hiddenFaces |= (diff.y < half) << (int)BLOCK_FACE::BOTTOM;

			hiddenFaces |= (diff.z > -half) << (int)BLOCK_FACE::FRONT;
			hiddenFaces |= (diff.z < half) << (int)BLOCK_FACE::BACK;*/

			if (hiddenFaces >= 63) continue;

			Instance* i = nullptr;
			for (auto& inst : instancesCache[hiddenFaces]) {
				if (inst->offsetsCount >= MAX_INSTANCE_OFFSETS) continue;
				if (inst->transparent != transparent) continue;
				i = inst;
				break;
			}
			if (i == nullptr) {
				i = new Instance();
				i->offsetsCount = 0;
				i->hiddenFaces = hiddenFaces;
				i->transparent = transparent;

				instancesMutex.lock();
				instances.push_back(i);
				instancesMutex.unlock();

				instancesToInitMutex.lock();
				instancesToInit.push_back(i);
				instancesToInitMutex.unlock();

				instancesCache[hiddenFaces].push_back(i);
			}

			i->offsets[i->offsetsCount++] = glm::vec4(bPos, type);
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
	skyboxShader->setUniform("view", glm::lookAt(glm::vec3(), player->orientation, player->up));
	skyboxShader->setUniform("projection", playerProjection);
	skyboxShader->setUniform("ambientLight", getAmbientLight());

	glDepthRange(0.99, 1.0);

	glBindTexture(GL_TEXTURE_2D, getTexture("skybox")->id);
	skyboxShader->setUniform("blockPos", glm::vec3(0, 0, 0));
	skyboxShader->setUniform("type", 0);
	skybox->Render(nullptr, 0, false);

	glDepthRange(0.95, 0.99);

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

	glDepthRange(0.9, .95);

	glBindTexture(GL_TEXTURE_2D, getTexture("clouds")->id);
	skyboxShader->setUniform("blockPos", glm::vec3(0, .75f, 0));
	skyboxShader->setUniform("playerPos", player->pos);
	skyboxShader->setUniform("type", 1);
	clouds->Render(nullptr, 47, false);

	glDepthRange(0, 0.9);

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

	// Camera Position
	glm::vec3 pos = player->getCameraPos();
	int renderDistance = Game::getInstance()->getRenderDistance();
	shader->setUniform("fogSize", 2.0f);
	shader->setUniform("playerPos", pos);

	float cosHalfFOV = std::cos(glm::radians(190.0f / 2.0f));

	glm::ivec2 playerChunk = getPosChunk(pos);

	shader->setUniform("ambientLight", getAmbientLight());

	static std::vector<Chunk*> chunksToRender;

	static glm::ivec2 oldPlayerChunk = playerChunk;
	static size_t oldChunksLoaded = 0;
	size_t chunksLoaded = this->chunksLoaded() - this->chunkLoadQueueCount();
	bool rerender = false;

	if (oldPlayerChunk != playerChunk || oldChunksLoaded != chunksLoaded) {
		chunksToRender.clear();

		for (int x = -renderDistance + playerChunk.x; x < renderDistance + playerChunk.x; x++) {
			for (int y = -renderDistance + playerChunk.y; y < renderDistance + playerChunk.y; y++) {
				glm::ivec2 cPos = glm::ivec2(x, y);

				if (glm::length(glm::vec2(cPos - playerChunk)) > renderDistance) continue;

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

				chunksToRender.push_back(chunk);
			}
		}

		oldPlayerChunk = playerChunk;
		oldChunksLoaded = chunksLoaded;
		rerender = true;
	}

	int c = 0;

	if (rerender || m_worldRenderModified) {
		for (auto& chunk : chunksToRender) {
			glm::ivec2 cPos = chunk->pos;

			m_chunksRendered++;

			renderingQueue.push_back(chunk);
			rendered[c] = false;

			c = (c + 1) % RENDERER_THREAD_COUNT;

			chunk->lastRendered = time(nullptr);
		}

		for (auto& i : instances) {
			i->offsetsCount = 0;
		}

		rendering.store(true);
		for (int c = 0; c < RENDERER_THREAD_COUNT; c++) {
			while (!rendered[c].load());
		}
		rendering.store(false);

		m_worldRenderModified = false;
	}

	std::partition(instances.begin(), instances.end(), [](const Instance* i) {
		return !i->transparent;
		});

	while (instancesToInit.size() != 0) {
		Instance* i = instancesToInit.front();
		instancesToInit.erase(instancesToInit.begin());

		BlockStructureData* data = createBlockStructureData(i->hiddenFaces);
		i->bStructData = data;

		glBindVertexArray(data->VAO);

		glGenBuffers(1, &i->VBO);
		glBindBuffer(GL_ARRAY_BUFFER, i->VBO);
		glBufferData(GL_ARRAY_BUFFER, MAX_INSTANCE_OFFSETS * sizeof(glm::vec4), nullptr, GL_DYNAMIC_DRAW);

		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), 0);
		glEnableVertexAttribArray(3);
		glVertexAttribDivisor(3, 1);
	}

	TextureAtlas* atlas = Game::getInstance()->getTexAtlas();
	glBindTexture(GL_TEXTURE_2D, atlas->tex->id);

	m_blocksRendered = 0;
	for (auto& i : instances) {
		if (i->offsetsCount == 0 || i->bStructData == nullptr) continue;

		glBindBuffer(GL_ARRAY_BUFFER, i->VBO);
		glBindVertexArray(i->bStructData->VAO);

		glBufferSubData(GL_ARRAY_BUFFER, 0, i->offsetsCount * sizeof(glm::vec4), i->offsets);

		glDrawElementsInstanced(GL_TRIANGLES, i->bStructData->faceCount * 6, GL_UNSIGNED_BYTE, 0, i->offsetsCount);

		m_instancesRendered++;
		m_blocksRendered += i->offsetsCount;
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}