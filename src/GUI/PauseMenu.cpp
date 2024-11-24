#include "PauseMenu.h"
#include "../Game/Game.h"

PauseMenu::PauseMenu() {
	title = new Text();
	title->setText("Game Paused");
	title->setPosition({ .5f, 0, .5f, -50 });
	title->setCentered(true);
	title->setScale(2.0f);

	resume = new Button();
	resume->setText("Resume");
	resume->setPosition({ .5f, 0, .5f, 0 });

	mainMenu = new Button();
	mainMenu->setText("Main Menu");
	mainMenu->setPosition({ .5f, 0, .5f, 50 });
}

PauseMenu::~PauseMenu() {
	delete title;
	title = nullptr;

	delete resume;
	resume = nullptr;

	delete mainMenu;
	mainMenu = nullptr;
}

void PauseMenu::render() {
	resume->render();
	mainMenu->render();
	title->render();

	if (resume->isClicked()) {
		Game::getInstance()->setGamePaused(false);
	}

	if (mainMenu->isClicked()) {
		Game::getInstance()->unloadWorld();
	}
}
