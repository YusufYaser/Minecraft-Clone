#pragma once

#include "../Components/Text.h"
#include "../Components/Button.h"
#include "../../World/World.h"
#include "WorldSelector.h"
#include "../SettingsMenu.h"

class MainMenu {
public:
	MainMenu();
	~MainMenu();

	void render();

private:
	Image* background;
	Text* title;
	Button* selectWorld;
	Button* settings;
	Button* quit;
	Text* credits;
#ifdef GAME_DEBUG
	Text* debug;
#endif

	WorldSelector* worldSelector = nullptr;
	SettingsMenu* settingsMenu = nullptr;
};
