#pragma once

#include "./Components/Text.h"
#include "./Components/Button.h"

class MainMenu {
public:
	MainMenu();
	~MainMenu();

	void render();

private:
	Text* title;
	Button* newWorld;
	Button* quit;
	Text* credits;
};
