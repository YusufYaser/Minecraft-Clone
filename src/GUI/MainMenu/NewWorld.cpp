#include "NewWorld.h"
#include "../../Game/Game.h"

std::string getAutoGotoWorld();

NewWorld::NewWorld() {
	background = new Image(getTexture("background"));
	background->setSize({ 1, 0, 1, 0 });
	background->setPosition({ .5f, 0, .5f, 0 });

	title = new Text();
	title->setText("Create a New World");
	title->setPosition({ .5f, 0, .5f, -75 });
	title->setCentered(true);
	title->setScale(2.5f);

	generatorText = new Text();
	generatorText->setText("Generator");
	generatorText->setPosition({ .5f, -136, .5f, -16 });
	generatorText->setCentered(true);

	defaultGenerator = new Button();
	defaultGenerator->setText("Default");
	defaultGenerator->setPosition({ .5f, 64 + -72 - 8, .5f, -16 });
	defaultGenerator->setSize({ 0, 72, 0, 24 });

	flatGenerator = new Button();
	flatGenerator->setText("Flat");
	flatGenerator->setPosition({ .5f, 64 + 0, .5f, -16 });
	flatGenerator->setSize({ 0, 72, 0, 24 });

	voidGenerator = new Button();
	voidGenerator->setText("Void");
	voidGenerator->setPosition({ .5f, 64 + 72 + 8, .5f, -16 });
	voidGenerator->setSize({ 0, 72, 0, 24 });

	structuresText = new Text();
	structuresText->setText("Structures");
	structuresText->setPosition({ .5f, -136, .5f, 16 });
	structuresText->setCentered(true);

	enableStructures = new Button();
	enableStructures->setText("Enabled");
	enableStructures->setPosition({ .5f, 64 - 60, .5f, 16 });
	enableStructures->setSize({ 0, 112, 0, 24 });

	disableStructures = new Button();
	disableStructures->setText("Disabled");
	disableStructures->setPosition({ .5f, 64 + 60, .5f, 16 });
	disableStructures->setSize({ 0, 112, 0, 24 });

	createWorldButton = new Button();
	createWorldButton->setText("Create World");
	createWorldButton->setPosition({ .5f, 0, 1, -124 });
	createWorldButton->setSize({ 0, 256 * 2 + 16, 0, 32 });

	back = new Button();
	back->setText("Back");
	back->setPosition({ .5f, 0, 1, -75 });
}

NewWorld::~NewWorld() {
}

void NewWorld::render() {
	static bool autoWentToWorld = false;
	Game* game = Game::getInstance();

	background->render();
	title->render();

	if (game->loadingWorld()) {
		createWorldButton->setText("Creating your brand new world");
	}

	defaultGenerator->setEnabled(selectedGenerator != Generator::Default);
	flatGenerator->setEnabled(selectedGenerator != Generator::Flat);
	voidGenerator->setEnabled(selectedGenerator != Generator::Void);

	enableStructures->setEnabled(!structuresEnabled);
	enableStructures->setText(structuresEnabled ? "Enabled" : "Enable");
	disableStructures->setEnabled(structuresEnabled);
	disableStructures->setText(!structuresEnabled ? "Disabled" : "Disable");

	generatorText->render();
	defaultGenerator->render();
	flatGenerator->render();
	voidGenerator->render();

	structuresText->render();
	enableStructures->render();
	disableStructures->render();

	createWorldButton->render();
	back->render();

	if (defaultGenerator->isClicked()) selectedGenerator = Generator::Default;
	if (flatGenerator->isClicked()) selectedGenerator = Generator::Flat;
	if (voidGenerator->isClicked()) selectedGenerator = Generator::Void;

	if (enableStructures->isClicked()) structuresEnabled = true;
	if (disableStructures->isClicked()) structuresEnabled = false;

	if (createWorldButton->isClicked() || (!autoWentToWorld && getAutoGotoWorld() == "NEW_WORLD")) {
		autoWentToWorld = true;

		WorldSettings settings;
		settings.generator = selectedGenerator;

		if (!structuresEnabled) {
			settings.structuresCount = 0;
			settings.allStructures = false;
			settings.structures = {};
		} else {
			settings.structuresCount = STRUCTURES_COUNT;
			settings.allStructures = true;
			for (int i = 0; i < settings.structuresCount; i++) {
				settings.structures[i] = (STRUCTURE_TYPE)i;
			}
		}

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

	createWorldButton->setEnabled(!game->loadingWorld());
	back->setEnabled(!game->loadingWorld());

	if ((back->isClicked() || game->getKeyHandler()->keyClicked(GLFW_KEY_ESCAPE)) && !game->loadingWorld()) {
		m_isClosing = true;
		return;
	}
}
