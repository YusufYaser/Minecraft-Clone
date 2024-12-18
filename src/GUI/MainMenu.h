#pragma once

#include "./Components/Text.h"
#include "./Components/Button.h"
#include "../World/World.h"

class MainMenu {
public:
	MainMenu();
	~MainMenu();

	void render();

private:
	Text* title;
	Button* newWorld;
	Button* worldType;
	Button* quit;
	Text* credits;

	Generator generator;
};
