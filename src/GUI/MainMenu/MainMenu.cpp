#include "MainMenu.h"
#include "../../Game/Game.h"

std::string getAutoGotoWorld();

MainMenu::MainMenu() {
	background = new Image(getTexture("background"));
	background->setSize({ 1, 0, 1, 0 });
	background->setPosition({ .5f, 0, .5f, 0 });

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
	credits->setText("Minecraft Clone - " GITHUB_REPOSITORY);
	credits->setPosition({ 0, 0, 1.0f, -15 });
	credits->setColor({ .75f, .75f, .75f, 1.0f });

	githubButton = new Button();
	githubButton->setPosition({ 0, 32, 1, -44 });
	githubButton->setSize({ 0, 48, 0, 48 });
	githubButton->setBackground(getTexture("github"));

#ifdef GAME_DEBUG
	debug = new Text();
	debug->setText("Debug Build");
	debug->setPosition({ .5f, 0, .5f, -75 + 18 });
	debug->setColor({ 1.0f, .2f, .2f, 1.0f });
	debug->setCentered(true);
#endif
}

MainMenu::~MainMenu() {
	delete background;
	background = nullptr;

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

	delete githubButton;
	githubButton = nullptr;

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

	background->render();
	selectWorld->render();
	settings->render();
	quit->render();
	title->render();
	credits->render();

	githubButton->render();

	if (githubButton->isClicked()) {
		std::string url = GITHUB_REPOSITORY;
		std::string cmd = "echo Platform not supported";
#ifdef _WIN32
		cmd = "explorer " + url;
#elif __APPLE__
		cmd = "open " + url;
#elif __linux__
		cmd = "xdg-open " + url;
#else
		error("Platform not supported");
#endif
		system(cmd.c_str());
	}

#ifdef GAME_DEBUG
	debug->render();
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
