#pragma once

#include "./Components/Text.h"
#include "./Components/Button.h"

class PauseMenu {
public:
	PauseMenu();
	~PauseMenu();

	void render();

private:
	Text* title;
	Button* resume;
	Button* quit;
};
