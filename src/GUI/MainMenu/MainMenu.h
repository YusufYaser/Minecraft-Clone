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
	Text* title;
	Button* selectWorld;
	Button* settings;
	Button* quit;
	Text* credits;
#ifdef _DEBUG
	Text* debug;
#endif

	WorldSelector* worldSelector;
	SettingsMenu* settingsMenu;
};
