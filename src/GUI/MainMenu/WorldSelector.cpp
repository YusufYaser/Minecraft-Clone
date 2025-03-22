#include "WorldSelector.h"
#include "../../Game/Game.h"
#include <filesystem>

std::string getAutoGotoWorld();

WorldSelector::WorldSelector() {
	background = new Image(getTexture("background"));
	background->setSize({ 1, 0, 1, 0 });
	background->setPosition({ .5f, 0, .5f, 0 });

	title = new Text();
	title->setText("Select World");
	title->setPosition({ .5f, 0, 0, 75 });
	title->setCentered(true);
	title->setScale(2.0f);

	newWorld = new Button();
	newWorld->setText("Create New World");
	newWorld->setPosition({ .5f, 0, .5f, 0 });
	newWorld->setSize({ 0, 512, 0, 64 });

	worldsDirectory = new Button();
	worldsDirectory->setText("Open Worlds Directory");
	worldsDirectory->setPosition({ .5f, 0, 1, -125 });

	back = new Button();
	back->setText("Back");
	back->setPosition({ .5f, 0, 1, -75 });

#ifdef GAME_DEBUG
	debugWorld = new Button();
	debugWorld->setText("Debug World");
	debugWorld->setPosition({ 1, -65, 1, -25 });
	debugWorld->setSize({ 0, 100, 0, 25 });
#endif

	overflow = new Text();
	overflow->setText("There are more worlds below");
	overflow->setCentered(true);
	overflow->setPosition({ .5f, 0, 1, -160 });
	overflow->setColor({ .75f, .75f, .75f, 1.0f });

	try {
		std::filesystem::create_directory("worlds");

		int i = 0;
		for (auto& dir : std::filesystem::directory_iterator("worlds")) {
			std::string p = dir.path().string();

			try {
				if (!dir.is_directory()) {
					warn("Ignoring unknown file:", p);
					continue;
				}

				if (std::filesystem::exists(p + "/world.dat")) {
					bool corrupted = false;

					if (std::filesystem::file_size(p + "/world.dat") < 24) {
						warn("Invalid world.dat size:", p);
						corrupted = true;
					}

					WorldEntry* world = new WorldEntry();
					world->name = dir.path().stem().string();


					world->playButton = new Button();
					world->playButton->setText(world->name.c_str());
					world->playButton->setEnabled(!corrupted);
					world->playButton->setPosition({ .5f, 0, 0, 125 + i * 40 });
					world->playButton->setSize({ 0, 384, 0, 32 });

					worlds.push_back(world);

					i++;
				} else {
					warn("Ignoring unknown directory:", p);
					continue;
				}
			} catch (std::filesystem::filesystem_error e) {
				error("Failed to load world:", e.what());
			}
		}
		if (i != 0) {
			newWorld->setSize({ 0, 256, 0, 32 });
			newWorld->setPosition({ .5f, -128 - 8, 1, -124 });
			worldsDirectory->setPosition({ .5f, 128 + 8, 1, -124 });
		}
	} catch (std::filesystem::filesystem_error e) {
		error("Failed to load worlds:", e.what());
	}
}

WorldSelector::~WorldSelector() {
	delete background;
	background = nullptr;

	delete title;
	title = nullptr;

	delete newWorld;
	newWorld = nullptr;

	delete back;
	back = nullptr;

	delete overflow;
	overflow = nullptr;

	worlds.clear();
}

void WorldSelector::render() {
	static bool autoWentToWorld = false;

	Game* game = Game::getInstance();
	if (game->loadingWorld()) {
		if (game->getWorld() != nullptr) {
			int progress = static_cast<int>((32 - game->getWorld()->chunkLoadQueueCount()) / 32.0f * 100);
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

	background->render();

	title->render();
	newWorld->render();
	worldsDirectory->render();
	back->render();
#ifdef GAME_DEBUG
	debugWorld->render();
#endif

	if (worldsDirectory->isClicked()) {
		std::filesystem::path dirPath = std::filesystem::current_path() / "worlds";
		std::string dir = dirPath.string();
		std::string cmd = "echo Platform not supported";
#ifdef _WIN32
		cmd = "explorer " + dir;
#elif __APPLE__
		cmd = "open " + dir;
#elif __linux__
		cmd = "xdg-open " + dir;
#else
		error("Platform not supported");
#endif
		system(cmd.c_str());
	}

	glm::ivec2 size = game->getGameWindow()->getSize();

	bool overflowed = false;
	for (auto& e : worlds) {
		if (e->playButton->getPosition().w > size.y - 185) {
			if (!overflowed) {
				overflow->setPosition(e->playButton->getPosition());
			}
			overflowed = true;
			continue;
		}

		e->playButton->render();
		if (game->loadingWorld()) e->playButton->setEnabled(false);

		bool autoGo = false;
		if (!autoWentToWorld && getAutoGotoWorld() != "") {
			if (e->name == getAutoGotoWorld()) {
				autoWentToWorld = true;
				autoGo = true;
			}
		}

		if (e->playButton->isClicked() || autoGo) {
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
				settings.name = e->name;
				settings.seed = data->seed;
				settings.generator = data->generator;
				settings.initialTick = data->tick;
				settings.structures.clear();
				if (data->structuresCount <= STRUCTURES_COUNT) {
					for (int i = 0; i < data->structuresCount; i++) {
						if (i < STRUCTURES_COUNT) {
							settings.structures.push_back(data->structures[i]);
						} else {
							warn("Invalid structure", std::to_string(i));
						}
					}
				} else {
					warn("Invalid structures count");
				}

				glm::vec3 pos = {};
				pos.x = data->playerPos[0];
				pos.y = data->playerPos[1];
				pos.z = data->playerPos[2];

				glm::vec3 orientation = {};
				orientation.x = data->playerOrientation[0];
				orientation.y = data->playerOrientation[1];
				orientation.z = data->playerOrientation[2];

				if (orientation.x == 0 && orientation.y == 0 && orientation.z == 0) {
					orientation.x = 1;
				}

				game->loadWorld(settings, pos, orientation, data->playerFlying);

				double start = glfwGetTime();
				while (game->getWorld() == nullptr && game->loadingWorld() && glfwGetTime() - start < .1f) {}

				if (game->loadingWorld() && game->getWorld() != nullptr) {
					glm::ivec2 playerChunk = getPosChunk(pos);

					for (int x = -2 + playerChunk.x; x < 2 + playerChunk.x; x++) {
						for (int y = -2 + playerChunk.y; y < 2 + playerChunk.y; y++) {
							game->getWorld()->loadChunk({ x, y, });
						}
					}
				}

				delete data;
			} catch (std::filesystem::filesystem_error e) {
				error("Failed to load world:", e.what());
			}
		}
	}

	if (overflowed) overflow->render();

	if (newWorld->isClicked() || (!autoWentToWorld && getAutoGotoWorld() == "NEW_WORLD")) {
		autoWentToWorld = true;

		WorldSettings settings;
		settings.generator = Generator::Default;

		time_t timestamp = time(nullptr);
		tm datetime;
#ifdef _WIN32
		localtime_s(&datetime, &timestamp);
#else
		localtime_r(&timestamp, &datetime);
#endif
		char name[21];

		strftime(name, sizeof(name), "%d %b %Y %H-%M-%S", &datetime);

		settings.name = std::string(name);
		game->loadWorld(settings);
	}

#ifdef GAME_DEBUG
	if (debugWorld->isClicked() || (!autoWentToWorld && getAutoGotoWorld() == "DEBUG_WORLD")) {
		autoWentToWorld = true;

		WorldSettings settings;
		settings.name = "[internal:debug]";
		settings.generator = Generator::Debug;
		settings.internalWorld = true;
		settings.seed = 1;
		settings.structures = {};

		game->loadWorld(settings, glm::vec3(), glm::vec3(1, 0, 0), true);
	}
#endif

	if (!autoWentToWorld && getAutoGotoWorld() != "" && !game->loadingWorld()) {
		error("World not found:", getAutoGotoWorld());
		autoWentToWorld = true;
	}

	if ((back->isClicked() || game->getKeyHandler()->keyHeld(GLFW_KEY_ESCAPE)) && !game->loadingWorld()) {
		m_isClosing = true;
		return;
	}
}
