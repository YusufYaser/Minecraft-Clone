#include "MainMenu.h"
#include "../../Game/Game.h"

std::string getAutoGotoWorld();

MainMenu::MainMenu() {
	title = new Text();
	title->setText("Minecraft Clone");
	title->setPosition({ .5f, 0, .5f, -75 });
	title->setCentered(true);
	title->setScale(2.5f);

	selectWorld = new Button();
	selectWorld->setText("Select World");
	selectWorld->setPosition({ .5f, 0, .5f, 0 });

	settings = new Button();
	settings->setText("Settings");
	settings->setPosition({ .5f, 0, .5f, 50 });

	quit = new Button();
	quit->setText("Quit Game");
	quit->setPosition({ .5f, 0, .5f, 100 });

	credits = new Text();
	credits->setText("Minecraft Clone - github.com/YusufYaser/Minecraft-Clone");
	credits->setPosition({ 0, 0, 1.0f, -15 });
	credits->setColor({ .75f, .75f, .75f, 1.0f });

#ifdef GAME_DEBUG
	debug = new Text();
	debug->setText("Debug Build");
	debug->setPosition({ 1.0f, -95, 1.0f, -15 });
	debug->setColor({ 1.0f, .2f, .2f, 1.0f });
#endif
}

MainMenu::~MainMenu() {
	delete title;
	title = nullptr;

	delete selectWorld;
	selectWorld = nullptr;

	delete settings;
	settings = nullptr;

	delete quit;
	quit = nullptr;

	delete credits;
	credits = nullptr;

#ifdef GAME_DEBUG
	delete debug;
	debug = nullptr;
#endif
}

void MainMenu::render() {
	static bool autoWentToWorld = false;
	if (!autoWentToWorld && getAutoGotoWorld() != "") {
		autoWentToWorld = true;
		worldSelector = new WorldSelector();
	}

	Game* game = Game::getInstance();
	static int lastFrame = 0;
	if (worldSelector != nullptr && (lastFrame + 1 != game->getFrameNum() || worldSelector->isClosing())) {
		delete worldSelector;
		worldSelector = nullptr;
	}
	if (settingsMenu != nullptr && (lastFrame + 1 != game->getFrameNum() || settingsMenu->isClosing())) {
		delete settingsMenu;
		settingsMenu = nullptr;
	}

	lastFrame = game->getFrameNum();

	if (worldSelector != nullptr) {
		worldSelector->render();
		return;
	}

	if (settingsMenu != nullptr) {
		settingsMenu->render();
		return;
	}

	selectWorld->render();
	settings->render();
	quit->render();
	title->render();
	credits->render();
#ifdef GAME_DEBUG
	if (static_cast<int>(glfwGetTime()) & 1) debug->render();
#endif

	if (selectWorld->isClicked()) {
		worldSelector = new WorldSelector();
	}

	if (settings->isClicked()) {
		settingsMenu = new SettingsMenu();
	}

	if (quit->isClicked()) {
		game->quit();
	}
}
