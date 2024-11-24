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

	quit = new Button();
	quit->setText("Quit Game");
	quit->setPosition({ .5f, 0, .5f, 50 });

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
		newWorld->setEnabled(false);
	} else {
		newWorld->setText("New World");
		newWorld->setEnabled(true);
	}

	newWorld->render();
	quit->render();
	title->render();
	credits->render();

	if (newWorld->isClicked()) {
		game->loadWorld();
	}

	if (quit->isClicked()) {
		game->quit();
	}
}
