#include "MainMenu.h"
#include "../../Game/Game.h"

MainMenu::MainMenu() {
	title = new Text();
	title->setText("Minecraft Clone");
	title->setPosition({ .5f, 0, .5f, -75 });
	title->setCentered(true);
	title->setScale(2.5f);

	selectWorld = new Button();
	selectWorld->setText("Select World");
	selectWorld->setPosition({ .5f, 0, .5f, 0 });

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

	delete selectWorld;
	selectWorld = nullptr;

	delete quit;
	quit = nullptr;

	delete credits;
	credits = nullptr;
}

void MainMenu::render() {
	Game* game = Game::getInstance();
	static int lastFrame = 0;
	if (worldSelector != nullptr && (lastFrame + 1 != game->getFrameNum() || worldSelector->isClosing())) {
		delete worldSelector;
		worldSelector = nullptr;
	}

	lastFrame = game->getFrameNum();

	if (worldSelector != nullptr) {
		worldSelector->render();
		return;
	}

	selectWorld->render();
	quit->render();
	title->render();
	credits->render();

	if (selectWorld->isClicked()) {
		worldSelector = new WorldSelector();
	}

	if (quit->isClicked()) {
		game->quit();
	}
}
