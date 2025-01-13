#include "World.h"
#include "../Game/Game.h"

void World::renderer(int c) {
	std::map<Texture*, std::vector<Instance*>> instancesCache[64];
	Player* player = nullptr;

	while (!unloading.load()) {
		if (!rendering.load()) continue;

		if (player == nullptr) player = Game::getInstance()->getPlayer();

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
				for (auto& inst : instancesCache[hiddenFaces][tex]) {
					if (inst->offsetsCount >= MAX_INSTANCE_OFFSETS) continue;
					if (inst->tex->id != tex->id) continue;
					if (inst->hiddenFaces != hiddenFaces) continue;
					i = inst;
					break;
				}
				if (i == nullptr) {
					i = new Instance();
					i->tex = tex;
					i->offsetsCount = 0;
					i->blockType = type;
					i->hiddenFaces = hiddenFaces;

					instancesMutex.lock();
					instances.push_back(i);
					instancesMutex.unlock();

					instancesToInitMutex.lock();
					instancesToInit.push_back(i);
					instancesToInitMutex.unlock();

					instancesCache[hiddenFaces][tex].push_back(i);
				}

				if (block->highlighted) i->highlightedOffset = i->offsetsCount;

				i->offsets[i->offsetsCount++] = glm::vec3(bPos);
			}
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

	glDepthRange(0.01, 0.9);


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

	for (auto& i : instances) {
		i->offsetsCount = 0;
		i->highlightedOffset = -1;
	}

	int c = 0;

	for (int x = -(renderDistance / 2) + playerChunk.x; x < (renderDistance / 2) + playerChunk.x; x++) {
		for (int y = -(renderDistance / 2) + playerChunk.y; y < (renderDistance / 2) + playerChunk.y; y++) {
			glm::ivec2 cPos = glm::ivec2(x, y);

			if (glm::length(glm::vec2(cPos - playerChunk)) > 1) {
				glm::vec3 chunkCenter = glm::vec3(x * 16 + 8, 20.0f, y * 16 + 8);
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
			m_chunksRendered++;

			renderingQueue.push_back(chunk);
			rendered[c] = false;

			c = (c + 1) % RENDERER_THREAD_COUNT;

			chunk->lastRendered = time(nullptr);
		}
	}

	rendering.store(true);
	for (int c = 0; c < RENDERER_THREAD_COUNT; c++) {
		while (!rendered[c].load());
	}
	rendering.store(false);

	std::partition(instances.begin(), instances.end(), [](const Instance* i) {
		return !isBlockTypeTransparent(i->blockType);
		});

	while (true) {
		if (instancesToInit.size() == 0) break;

		Instance* i = instancesToInit.front();
		instancesToInit.erase(instancesToInit.begin());

		BlockStructureData* data = createBlockStructureData(i->hiddenFaces);
		i->bStructData = data;

		glBindVertexArray(data->VAO);

		glGenBuffers(1, &i->VBO);
		glBindBuffer(GL_ARRAY_BUFFER, i->VBO);
		glBufferData(GL_ARRAY_BUFFER, MAX_INSTANCE_OFFSETS * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
		glEnableVertexAttribArray(3);
		glVertexAttribDivisor(3, 1);
	}

	for (auto& i : instances) {
		if (i->offsetsCount == 0 || i->bStructData == nullptr) continue;
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

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	if (targetBlock != nullptr) targetBlock->highlighted = false;
}