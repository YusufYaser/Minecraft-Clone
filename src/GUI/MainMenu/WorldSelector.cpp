#include "WorldSelector.h"
#include "../../Game/Game.h"
#include <filesystem>

WorldSelector::WorldSelector() {
	title = new Text();
	title->setText("Select World");
	title->setPosition({ .5f, 0, 0, 75 });
	title->setCentered(true);
	title->setScale(2.0f);

	newWorld = new Button();
	newWorld->setText("Create New World");
	newWorld->setPosition({ .5f, 0, .5f, 0 });
	newWorld->setSize({ 0, 512, 0, 64 });

	back = new Button();
	back->setText("Back");
	back->setPosition({ .5f, 0, 1, -75 });

	int i = 0;
	for (auto& dir : std::filesystem::directory_iterator("worlds")) {
		std::string p = dir.path().string();

		if (!dir.is_directory()) {
			warn("Ignoring unknown file:", p);
			continue;
		}

		if (std::filesystem::exists(p + "/world.dat")) {
			if (std::filesystem::file_size(p + "/world.dat") != sizeof(WorldSaveData)) {
				warn("Ignoring world with invalid world data size:", p);
				continue;
			}

			WorldEntry* world = new WorldEntry();
			world->name = dir.path().stem().string();


			world->playButton = new Button();
			world->playButton->setText(world->name.c_str());
			world->playButton->setPosition({ .5f, 0, 0, 125 + i * 40 });
			world->playButton->setSize({ 0, 384, 0, 32 });

			worlds.push_back(world);

			i++;
		} else {
			warn("Ignoring unknown directory:", p);
			continue;
		}
	}
	if (i != 0) {
		newWorld->setSize({ 0, 256, 0, 32 });
		newWorld->setPosition({ .5f, 0, 1, -125 });
	}
}

WorldSelector::~WorldSelector() {
	delete title;
	title = nullptr;

	delete newWorld;
	newWorld = nullptr;

	delete back;
	back = nullptr;

	worlds.clear();
}

void WorldSelector::render() {
	Game* game = Game::getInstance();
	if (game->loadingWorld()) {
		if (game->getWorld() != nullptr) {
			int progress = static_cast<int>((16 - game->getWorld()->chunkLoadQueueCount()) / 16.0f * 100);
			std::stringstream str;
			str << "Loading World (" << progress << "%)";
			newWorld->setText(str.str().c_str()); // str :P
		} else {
			newWorld->setText("Loading World");
		}
	} else {
		newWorld->setText("Create New World");
	}

	newWorld->setEnabled(!game->loadingWorld());
	back->setEnabled(!game->loadingWorld());

	title->render();
	newWorld->render();
	back->render();

	for (auto& e : worlds) {
		e->playButton->render();
		e->playButton->setEnabled(!game->loadingWorld());

		if (e->playButton->isClicked()) {
			try {
				print("Loading world", e->name);
				WorldSaveData* data = new WorldSaveData();

				std::ifstream dataFile("worlds/" + e->name + "/world.dat", std::ios::binary);
				if (!dataFile.is_open()) {
					error("Failed to open world.dat");
					continue;
				}
				dataFile.read(reinterpret_cast<char*>(data), sizeof(WorldSaveData));
				dataFile.close();

				WorldSettings settings;
				settings.seed = data->seed;
				settings.generator = data->generator;
				settings.initialTick = data->tick;
				settings.structures.clear();
				for (int i = 0; i < data->structuresCount; i++) {
					settings.structures.push_back(data->structures[i]);
				}

				glm::vec3 pos = {};
				pos.x = data->playerPos[0];
				pos.y = data->playerPos[1];
				pos.z = data->playerPos[2];

				game->setLoadedWorldName(e->name);
				game->loadWorld(settings, pos);

				delete data;
			} catch (std::filesystem::filesystem_error e) {
				error("Failed to load world:", e.what());
			}
		}
	}

	if (newWorld->isClicked()) {
		WorldSettings settings;
		settings.generator = Generator::Default;

		time_t timestamp = time(nullptr);
		tm datetime;
		localtime_s(&datetime, &timestamp);
		char name[21];

		strftime(name, sizeof(name), "%d %b %Y %H-%M-%S", &datetime);

		game->setLoadedWorldName(&name[0]);
		game->loadWorld(settings);
	}

	if (back->isClicked() || game->getKeyHandler()->keyHeld(GLFW_KEY_ESCAPE)) {
		m_isClosing = true;
		return;
	}
}