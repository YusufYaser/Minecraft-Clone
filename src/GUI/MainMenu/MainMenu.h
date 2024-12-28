#pragma once

#include "../Components/Text.h"
#include "../Components/Button.h"
#include "../../World/World.h"
#include "WorldSelector.h"

class MainMenu {
public:
	MainMenu();
	~MainMenu();

	void render();

private:
	Text* title;
	Button* selectWorld;
	Button* quit;
	Text* credits;
#ifdef _DEBUG
	Text* debug;
#endif

	WorldSelector* worldSelector;
};
