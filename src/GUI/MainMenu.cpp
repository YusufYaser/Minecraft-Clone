#include "MainMenu.h"
#include "../Game/Game.h"

MainMenu::MainMenu() {
	title = new Text();
	title->setText("Minecraft Clone");
	title->setPosition({ .5f, 0, .5f, -75 });
	title->setCentered(true);
	title->setScale(2.5f);

	newWorld = new Button();
	newWorld->setText("New World");
	newWorld->setPosition({ .5f, 0, .5f, 0 });

	worldType = new Button();
	worldType->setText("World Type: Default");
	worldType->setPosition({ .5f, 0, .5f, 50 });
	generator = Generator::Default;

	quit = new Button();
	quit->setText("Quit Game");
	quit->setPosition({ .5f, 0, .5f, 100 });

	credits = new Text();
	credits->setText("Minecraft Clone - github.com/YusufYaser/Minecraft-Clone");
	credits->setPosition({ 0, 0, 1.0f, -15 });
	credits->setColor({ .75f, .75f, .75f, 1.0f });
}

MainMenu::~MainMenu() {
	delete title;
	title = nullptr;

	delete newWorld;
	newWorld = nullptr;
}

void MainMenu::render() {
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
		newWorld->setText("New World");
	}

	newWorld->setEnabled(!game->loadingWorld());
	worldType->setEnabled(!game->loadingWorld());

	newWorld->render();
	worldType->render();
	quit->render();
	title->render();
	credits->render();

	if (newWorld->isClicked()) {
		WorldSettings settings;
		settings.generator = generator;
		game->loadWorld(settings);
	}

	if (worldType->isClicked()) {
		generator = Generator(int(generator) + 1);
		if (int(generator) >= 3) generator = Generator(0);

		const char* name = "Invalid";
		switch (generator) {
		case Generator::Default:
			name = "Default";
			break;
		case Generator::Flat:
			name = "Flat";
			break;
		case Generator::Void:
			name = "Void";
			break;
		}

		std::string worldTypeText = "World Type: " + std::string(name);
		worldType->setText(worldTypeText.c_str());
	}

	if (quit->isClicked()) {
		game->quit();
	}
}
