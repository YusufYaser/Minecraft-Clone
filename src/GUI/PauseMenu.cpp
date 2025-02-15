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

	settings = new Button();
	settings->setText("Settings");
	settings->setPosition({ .5f, 0, .5f, 50 });

	screenshot = new Button();
	screenshot->setText("Take Photo");
	screenshot->setPosition({ 1, -65, 1, -25 });
	screenshot->setSize({ 0, 100, 0, 25 });

	mainMenu = new Button();
	mainMenu->setText("Main Menu");
	mainMenu->setPosition({ .5f, 0, .5f, 100 });
}

PauseMenu::~PauseMenu() {
	delete title;
	title = nullptr;

	delete resume;
	resume = nullptr;

	delete settings;
	settings = nullptr;

	delete mainMenu;
	mainMenu = nullptr;
}

void PauseMenu::render() {
	Game* game = Game::getInstance();
	static int lastFrame = 0;
	if (settingsMenu != nullptr && (lastFrame + 1 != game->getFrameNum() || settingsMenu->isClosing())) {
		delete settingsMenu;
		settingsMenu = nullptr;
	}
	if (screenshotMenu != nullptr && (lastFrame + 1 != game->getFrameNum() || screenshotMenu->isClosing())) {
		delete screenshotMenu;
		screenshotMenu = nullptr;
	}

	lastFrame = game->getFrameNum();

	if (settingsMenu != nullptr) {
		settingsMenu->render();
		return;
	}

	if (screenshotMenu != nullptr) {
		screenshotMenu->render();
		return;
	}

	resume->render();
	settings->render();
	mainMenu->render();
	screenshot->render();
	title->render();

	if (resume->isClicked()) {
		Game::getInstance()->setGamePaused(false);
	}

	if (settings->isClicked()) {
		settingsMenu = new SettingsMenu();
	}

	if (screenshot->isClicked()) {
		screenshotMenu = new ScreenshotMenu();
	}

	if (mainMenu->isClicked()) {
		Game::getInstance()->unloadWorld();
	}
}
