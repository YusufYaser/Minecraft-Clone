#pragma once

#include "./Components/Text.h"
#include "./Components/Button.h"
#include "SettingsMenu.h"
#include "ScreenshotMenu.h"

class PauseMenu {
public:
	PauseMenu();
	~PauseMenu();

	bool onSettings() const { return settingsMenu != nullptr; }

	void render();

private:
	Text* title;
	Button* resume;
	Button* settings;
	Button* screenshot;
	Button* mainMenu;
	Button* newTempWorld;

	SettingsMenu* settingsMenu = nullptr;
	ScreenshotMenu* screenshotMenu = nullptr;
};
